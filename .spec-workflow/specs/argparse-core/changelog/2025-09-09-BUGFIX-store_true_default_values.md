# Bug Fix Report: store_true アクションのデフォルト値問題

**ファイル**: `2025-09-09-BUGFIX-store_true_default_values.md`  
**日付**: 2025-09-09  
**カテゴリ**: Bug Fix  
**承認ID**: `approval_1757403146748_6lkmu6987`  

## 問題の概要

store_trueアクションを使用した引数がコマンドラインで指定されない場合、`Namespace.get<bool>()`メソッドで`ArgumentError`例外が発生していました。これにより、ユーザーは必ずデフォルト値を明示的に指定する必要があり、使い勝手が悪化していました。

## 問題の詳細

### 再現手順
```cpp
argparse::ArgumentParser parser("test");
parser.add_argument("-v", "--verbose").action("store_true");
auto args = parser.parse_args({}); // 空の引数リスト

// 以下でランタイムエラーが発生
bool verbose = args.get<bool>("verbose"); // Exception: Argument 'verbose' not found
```

### 根本原因
- store_trueアクションの引数が指定されない場合、Namespaceに値が設定されない
- `Namespace.get<T>()`メソッドが値の存在チェックを行い、存在しない場合に例外を投げる
- Python argparseでは、store_trueアクションが指定されない場合に自動的にfalseが設定される

## 修正内容

### 1. 要件定義の更新 (`requirements.md`)
```markdown
2. WHEN actionパラメータで"store_true"を指定 THEN システムは フラグとして動作し、指定されない場合はfalseをデフォルト値として使用する
```

### 2. 設計書の更新 (`design.md`)
- Parserクラスにデフォルト値設定の詳細処理順序を追加
- `_set_explicit_default_values()`, `_set_boolean_action_defaults()`, `_validate_required_arguments()`の説明

### 3. 実装の修正 (`include/argparse/argparse.hpp`)

#### デフォルト値設定プロセスの分離
```cpp
// 旧: _set_default_values() - 単一メソッド
void _set_default_values(Namespace& result, const std::vector<std::shared_ptr<Argument>>& arguments);

// 新: 2段階のデフォルト値設定
void _set_explicit_default_values(Namespace& result, const std::vector<std::shared_ptr<Argument>>& arguments);
void _set_boolean_action_defaults(Namespace& result, const std::vector<std::shared_ptr<Argument>>& arguments);
```

#### 処理順序の変更
1. 明示的なデフォルト値を設定（store_true/store_falseは除く）
2. 引数を解析
3. **必須引数をバリデーション**
4. **store_true/store_falseアクションの暗黙デフォルト値を設定**

この順序により、必須引数のチェックは正常に動作し、かつstore_trueアクションで例外が発生しない仕様を実現。

### 4. テストの修正 (`tests/unit/required_test.cpp`)
Python argparseとの互換性を確保するため、`RequiredBooleanFlags`テストを修正：
```cpp
// store_trueアクションが指定されない場合
EXPECT_TRUE(ns.has("quiet"));        // quietには false が設定されるのでhas()はtrue
EXPECT_FALSE(ns.get<bool>("quiet")); // quietの値はfalse
```

## 検証結果

### Python argparse との互換性確認
```python
parser.add_argument('--quiet', '-q', action='store_true', required=False)
args = parser.parse_args(['--verbose'])  # quietは指定されない
print(f"quiet: {args.quiet}")           # False
print(f"'quiet' in vars(args): {'quiet' in vars(args)}")  # True
```

### テスト結果
- 全21テストが100%通過
- store_trueアクションが指定されない場合、`get<bool>()`は`false`を返し、例外は発生しない
- 必須引数の検証も正しく動作
- Python argparseと同じ動作を実現

## 影響範囲

### 破壊的変更
- なし（既存の動作を改善するのみ）

### 改善された動作
- store_trueアクションでデフォルト値の明示的指定が不要
- Python argparseとの完全互換性
- より直感的なAPI使用体験

## タスク文書の更新

- タスク11「アクション処理の実装」: Fix Appliedコメント追加
- タスク18「required引数の処理」: Fix Appliedコメント追加

## 承認ID
`approval_1757403146748_6lkmu6987`

## 修正日時
2025-09-09

## 修正者
Claude Code AI Assistant