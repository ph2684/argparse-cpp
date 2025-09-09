# プロジェクト構造

## ディレクトリ構成

```
argparse-cpp/
├── include/
│   └── argparse/
│       └── argparse.hpp        # メインヘッダファイル（単一ファイル実装）
├── tests/                       # テストファイル
│   ├── unit/                    # 単体テスト
│   │   ├── parser_test.cpp
│   │   ├── argument_test.cpp
│   │   └── subcommand_test.cpp
│   ├── integration/             # 統合テスト
│   │   └── python_compat_test.cpp
│   └── performance/             # パフォーマンステスト
│       └── benchmark.cpp
├── examples/                    # 使用例
│   ├── basic/                   # 基本的な使用例
│   │   ├── hello_world.cpp
│   │   └── positional_args.cpp
│   ├── advanced/                # 高度な使用例
│   │   ├── subcommands.cpp
│   │   └── custom_types.cpp
│   └── migration/               # Python argparseからの移行例
│       └── python_to_cpp.cpp
├── docs/                        # ドキュメント
│   ├── api/                     # APIリファレンス
│   ├── tutorial/                # チュートリアル
│   └── migration_guide.md      # 移行ガイド
├── scripts/                     # ユーティリティスクリプト
│   ├── generate_single_header.py
│   └── run_tests.sh
└── benchmarks/                  # ベンチマーク結果とスクリプト
    ├── results/
    └── compare_with_others.py
```

## 命名規則

### ファイル
- **ヘッダファイル**: `snake_case.hpp`（例：`argparse.hpp`）
- **実装ファイル**: `snake_case.cpp`（例：`parser_test.cpp`）
- **テストファイル**: `[対象]_test.cpp`（例：`argument_test.cpp`）
- **サンプルファイル**: `descriptive_name.cpp`（例：`hello_world.cpp`）

### コード
- **クラス/構造体**: `PascalCase`（例：`ArgumentParser`、`Argument`）
- **関数/メソッド**: `snake_case`（例：`add_argument()`、`parse_args()`）
- **定数**: `UPPER_SNAKE_CASE`（例：`DEFAULT_HELP_FLAG`、`MAX_ARGUMENTS`）
- **変数**: `snake_case`（例：`arg_name`、`parsed_values`）
- **テンプレートパラメータ**: `PascalCase`（例：`ValueType`、`Validator`）
- **ネームスペース**: `snake_case`（例：`argparse`、`internal`）

## インポートパターン

### インクルード順序
1. 標準ライブラリヘッダ
2. サードパーティライブラリ（テスト時のみ）
3. プロジェクト内ヘッダ

### 例：
```cpp
// 標準ライブラリ
#include <string>
#include <vector>
#include <memory>

// サードパーティ（テスト時のみ）
#include <gtest/gtest.h>

// プロジェクト内
#include <argparse/argparse.hpp>
```

### モジュール/パッケージ構成
- 単一ヘッダファイル`argparse.hpp`に全実装を含む
- 内部実装は`argparse::detail`ネームスペースで隠蔽
- 公開APIは`argparse`ネームスペース直下

## コード構造パターン

### ヘッダファイル構成
```cpp
// 1. インクルードガード/pragma once（unix/MSVC両対応）
#pragma once

#ifndef ARGPARSE_HPP_INCLUDED
#define ARGPARSE_HPP_INCLUDED

// 2. 標準ライブラリインクルード
#include <string>
#include <vector>

// 3. マクロ定義（必要最小限）
#define ARGPARSE_VERSION "0.1.0"

// 4. ネームスペース開始
namespace argparse {

// 5. 前方宣言
class Argument;

// 6. 型定義/エイリアス
using ArgumentList = std::vector<Argument>;

// 7. 主要クラス定義
class ArgumentParser {
    // Public API
public:
    // コンストラクタ/デストラクタ
    // 主要メソッド（Python argparse互換）
    
    // Private実装
private:
    // 内部データ構造
    // ヘルパーメソッド
};

// 8. インライン実装
// 9. 内部実装詳細（detail namespace）
namespace detail {
    // 実装詳細
}

} // namespace argparse

#endif // ARGPARSE_HPP_INCLUDED
```

### クラス構成原則
- **公開API優先**: Python argparse互換メソッドを最上部に配置
- **RAII原則**: リソース管理は自動化
- **例外安全性**: 強い例外安全性保証を提供
- **const正確性**: 適切なconst修飾子の使用

### 関数/メソッド構成
```cpp
template<typename T>
T ArgumentParser::get(const std::string& name) const {
    // 1. 入力検証
    validate_argument_name(name);
    
    // 2. 主要ロジック
    auto it = arguments_.find(name);
    if (it == arguments_.end()) {
        throw std::runtime_error("Unknown argument: " + name);
    }
    
    // 3. 型変換と返却
    return it->second.get_value<T>();
}
```

## コード構成原則

1. **単一責任原則**: 各クラスは明確な単一の責任を持つ
2. **Python互換性優先**: Python argparseのAPIとの一貫性を最優先
3. **ゼロオーバーヘッド原則**: 使用しない機能はコストを発生させない
4. **テンプレートメタプログラミング**: コンパイル時の型安全性確保

## モジュール境界

### コア vs 拡張
- **コア機能**: `ArgumentParser`、`Argument`基本クラス
- **拡張機能**: カスタムバリデータ、型変換器

### 公開API vs 内部実装
- **公開API**: `argparse`ネームスペース直下
- **内部実装**: `argparse::detail`ネームスペース内

### プラットフォーム固有 vs クロスプラットフォーム
- **クロスプラットフォーム**: 全主要機能
- **プラットフォーム固有**: ターミナル幅検出など（条件付きコンパイル）

## コードサイズガイドライン

- **ファイルサイズ**: 単一ヘッダのため、3000行以内を目標
- **関数/メソッドサイズ**: 最大50行（複雑なロジックは分割）
- **クラス複雑度**: 1クラスあたり最大20メソッド
- **ネスト深度**: 最大4レベル
- **テンプレート再帰深度**: 最大16レベル

## テスト構造

### 単体テスト構成
```cpp
// tests/unit/argument_test.cpp
#include <gtest/gtest.h>
#include <argparse/argparse.hpp>

TEST(ArgumentTest, BasicParsing) {
    // Arrange
    argparse::ArgumentParser parser("test");
    
    // Act
    parser.add_argument("--flag");
    
    // Assert
    EXPECT_TRUE(parser.has_argument("--flag"));
}
```

### 統合テスト構成
- Python argparseとの出力比較
- 実際のCLIアプリケーションシミュレーション
- エラーメッセージの一貫性確認

## ドキュメント標準

- **公開API**: Doxygenスタイルのコメント必須
- **複雑なロジック**: インラインコメントで説明
- **使用例**: 各主要機能に対してexamplesディレクトリに実例
- **Python互換性**: Python argparseとの違いを明記

### Doxygenコメント例
```cpp
/**
 * @brief コマンドライン引数を解析する
 * 
 * @param argc 引数の数
 * @param argv 引数の配列
 * @return 解析結果を含むNamespace オブジェクト
 * @throws std::runtime_error 解析エラー時
 * 
 * @note Python argparseのparse_args()と互換
 */
Namespace parse_args(int argc, const char* const argv[]);
```