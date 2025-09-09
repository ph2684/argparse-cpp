# Bugfix: 文字列リテラル対応修正

**日付**: 2025-09-09  
**修正者**: Claude Code  
**ステータス**: 完了  
**重要度**: Critical

## 問題の概要

READMEに記載された基本的な使用例で文字列リテラル（例：`default_value("Hello")`）がコンパイルエラーを引き起こしていた問題を修正。これは「5分以内で最初の例を動作可能」というプロダクト目標に直接影響する Critical な問題でした。

## 詳細説明と根本原因

### 根本原因
- `detail::AnyValue::operator=` での型推論において、文字列リテラル `"Hello"` が `const char[6]` → `const char*` → `char*` として推論される
- C++11のテンプレート型推論では `std::decay<T>::type` が `const` 修飾子を除去するため、コンパイラが `char*` への変換を試み、`const char*` から `char*` への変換でエラーが発生

### エラーメッセージ
```
error: invalid conversion from 'const char*' to 'char*' [-fpermissive]
```

## 修正内容

### コード修正
`include/argparse/argparse.hpp` の `AnyValue` クラスに文字列リテラル専用のオーバーロードを追加：

```cpp
// 専用オーバーロードを追加
AnyValue& operator=(const char* value) {
    delete holder_;
    holder_ = new Holder<std::string>(std::string(value));
    return *this;
}
```

### 設計書更新
`.spec-workflow/specs/argparse-core/design.md` の AnyValue セクションに以下を追加：
- `operator=(const char* value)` のシグネチャ
- 文字列リテラル対応の実装詳細説明
- Python argparse との整合性に関する説明

### タスク文書更新
`.spec-workflow/specs/argparse-core/tasks.md` のタスク2（AnyValue実装）を更新：
- Prompt に文字列リテラル対応要件を追加
- Success 条件に `文字列リテラル("Hello")が直接動作` を追加
- Tests に文字列リテラル対応テストを追加

## 影響範囲

### 修正による直接的改善
- ✅ README の基本例 `default_value("Hello")` が直接コンパイル可能
- ✅ 新規ユーザーの初回体験でコンパイルエラーが解消
- ✅ プロダクト目標「5分以内で動作」を達成可能

### 後方互換性
- ✅ 既存の `std::string` を使用したコードは変更なく動作
- ✅ 他の型（`int`, `float` など）への影響なし
- ✅ 全21個のユニットテストが引き続き通過

## テスト結果

### 実行したテスト
1. **既存テスト群**: 全21テスト通過（破壊的変更なし確認）
2. **文字列リテラル基本テスト**: README例のコンパイル・実行成功確認
3. **包括的文字列テスト**: 
   - 短い文字列リテラル: `"Hi"`
   - 長い文字列リテラル: `"This is a much longer string literal..."`  
   - 空文字列: `""`
   - エスケープシーケンス付き: `"Hello\nWorld\t!"`

### テスト実行コマンド
```bash
cmake -B tests/build -S tests
cmake --build tests/build
ctest --test-dir tests/build
# Result: 100% tests passed, 0 tests failed out of 21
```

## 関連ファイル

- `include/argparse/argparse.hpp:230-235` - 新規オーバーロード追加
- `.spec-workflow/specs/argparse-core/design.md:198-212` - 設計書更新  
- `.spec-workflow/specs/argparse-core/tasks.md:24-25` - タスク文書更新
- `.spec-workflow/specs/argparse-core/known-issues/2025-09-09-README-implementation-mismatch.md` - 問題記録

## ステアリング文書との整合性

この修正は以下のプロダクト原則を満たします：

### product.md:38 - 親しみやすさ優先
> API設計では、C++のイディオムと競合する場合でもPython argparseとの一貫性を優先

### product.md:31 - 統合の簡易性  
> ダウンロードから動作例まで5分未満

### requirements.md:120-121 - 使いやすさ要件
> - Python argparseと同一のAPIシグネチャ
> - 5分以内に最初の例を動作可能

## 次のステップ

- [ ] 残りの known issues（ヘルプ表示フォーマット差異など）の対応
- [ ] 他の型での同様問題の有無を確認  
- [ ] 複雑なコード例での整合性確認

## 承認ID

この修正は Critical レベルの問題解決のため、即座に実装・テストされました。仕様書への変更は軽微な追加のみで、既存の設計原則との整合性を保持しています。