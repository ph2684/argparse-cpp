# CLAUDE.md

このファイルは、このリポジトリのコードを使用してClaudeコード（claude.ai/code）にガイダンスを提供します。

## AI Instructions

英語で思考し、日本語で回答します。Think in English, answer in Japanese.

## プロジェクト概要

argparse-cppは、PythonのargparseモジュールのコアAPIをC++で実装するヘッダオンリーライブラリです。単一ファイル`include/argparse/argparse.hpp`によるC++11準拠の実装を通じて、Python開発者が慣れ親しんだAPIでC++のコマンドライン引数解析を可能にします。

## Spec Driven Development（SDD）

このプロジェクトはspec-workflow-mcpを使用したSpec Driven Development（SDD）アプローチを採用しています：

- `.spec-workflow/steering/`: プロダクト方針、技術スタック、プロジェクト構造の定義
- `.spec-workflow/specs/argparse-core/`: 要件定義、設計書、タスク管理

**重要**: 新機能の実装前に必ず関連する仕様書を参照し、spec-workflowツールを使用してタスクを管理してください。

## 開発コマンド

### テスト
```bash
# 単体テストの実行（CMakeベース - 将来実装予定）
mkdir build && cd build
cmake ..
make test

# Google Testベースの個別テスト実行
./build/tests/unit/parser_test
```

### ビルド
```bash
# サンプルのビルド（CMakeベース - 将来実装予定） 
mkdir build && cd build
cmake ..
make examples

# 手動コンパイル（現在の方法）
g++ -std=c++11 -I include examples/basic/hello_world.cpp -o hello_world
```

### 静的解析とフォーマット
```bash
# clang-tidyによる静的解析
clang-tidy include/argparse/argparse.hpp -- -std=c++11

# clang-formatによるフォーマット
clang-format -i include/argparse/argparse.hpp
```

## 高レベルアーキテクチャ

### ヘッダオンリー設計
- **単一ファイル**: `include/argparse/argparse.hpp`に全実装を含む
- **ネームスペース**: `argparse`（公開API）、`argparse::detail`（内部実装）
- **依存関係**: C++11標準ライブラリのみ（外部依存なし）

### コアコンポーネント

```
ArgumentParser (メインAPI)
├── ArgumentGroup (引数のグループ化)
├── Argument (個々の引数定義) 
├── Namespace (解析結果の保持)
└── detail::
    ├── AnyValue (型消去実装)
    ├── Parser (解析ロジック)
    ├── Tokenizer (引数の前処理)
    ├── TypeConverter (型変換)
    └── HelpGenerator (ヘルプ生成)
```

### Python argparse API互換性
- `ArgumentParser(prog, description, epilog)`
- `add_argument(name, action, type, default, help, ...)`
- `add_argument_group(title, description)`
- `parse_args(argc, argv)` / `parse_args(args)`
- `Namespace.get<T>(name)`でのタイプセーフなアクセス

## 重要な設計原則

1. **Python互換性優先**: APIシグネチャとパラメータ名をPython argparseと一致させる
2. **ゼロオーバーヘッド**: 使用しない機能はコンパイルされない
3. **型安全性**: C++のテンプレートシステムを活用した安全な実装
4. **例外安全**: RAIIと強い例外安全性保証
5. **明確なエラー**: Pythonと同様の実用的なエラーメッセージ

## 開発の注意点

### 型消去の実装
C++11でstd::anyが使用できないため、`argparse::detail::AnyValue`で型消去を実装:
```cpp
// 任意の型を保持
AnyValue value = std::string("test");
// 型安全な取得  
auto str = value.get<std::string>();
```

### エラー処理
- `std::runtime_error`: 必須引数欠落、不明なオプション
- `std::invalid_argument`: 型変換失敗、不正な選択肢
- Pythonと同様のエラーメッセージフォーマット

### テストアプローチ
- 単体テスト: 各コンポーネントの独立したテスト
- 統合テスト: Python argparseとの出力比較
- パフォーマンステスト: 引数100個で1ms未満の解析

## ファイル構造（実装予定）

```
include/argparse/argparse.hpp    # メインヘッダ（全実装）
tests/unit/                      # Google Test単体テスト
tests/integration/               # Python互換性テスト
examples/basic/                  # 基本使用例
examples/advanced/               # 高度な使用例
```

## コーディング標準

- **命名**: クラス=PascalCase、メソッド=snake_case、定数=UPPER_SNAKE_CASE
- **インクルード順序**: 標準ライブラリ → サードパーティ → プロジェクト内
- **C++11準拠**: 古いコンパイラでもビルド可能
- **const正確性**: 適切なconst修飾子の使用
- **Doxygenコメント**: 公開APIには必須

## spec-workflowとの連携

タスクの作業前に以下を実行:
```bash
# 仕様の確認
mcp__spec-workflow__get-spec-context argparse-core

# 次のタスク取得
mcp__spec-workflow__manage-tasks action=next-pending

# 実装後のタスク更新
mcp__spec-workflow__manage-tasks action=set-status taskId=<id> status=completed
```