# argparse-cpp

PythonのargparseモジュールのコアAPIをC++で実装したヘッダオンリーライブラリです。Python開発者が慣れ親しんだAPIを提供することで、C++でのコマンドライン引数解析を直感的かつ効率的に行えます。

## 特徴

- **Python argparse API互換性**: Pythonユーザーが学習コストゼロで使用開始可能
- **ヘッダオンリー設計**: 単一ファイルインクルードで即座に使用可能
- **型安全な解析**: C++の型システムを活用した安全な実装
- **C++11準拠**: 現代的なC++機能を活用しながら幅広い環境で利用可能
- **外部依存なし**: 標準ライブラリのみを使用

## インストール

このライブラリはヘッダオンリーのため、複数の方法で簡単に導入できます。

### 方法1: ヘッダファイルのコピー

最もシンプルな方法です。

```bash
# プロジェクトディレクトリに移動
cd your-project

# ヘッダファイルをコピー
cp /path/to/argparse-cpp/include/argparse/argparse.hpp include/
```

### 方法2: Git Submodule

プロジェクトで継続的にアップデートを受け取りたい場合は、Git submoduleを使用できます。

```bash
# サブモジュールとして追加
git submodule add https://github.com/your-username/argparse-cpp.git third_party/argparse-cpp

# サブモジュールを初期化・更新
git submodule update --init --recursive
```

その後、CMakeLists.txtで以下のように設定：

```cmake
# インクルードパスを追加
target_include_directories(your_target PRIVATE third_party/argparse-cpp/include)
```

### 方法3: CMake FetchContent

CMake 3.11以降をお使いの場合、FetchContentを使用できます。

```cmake
cmake_minimum_required(VERSION 3.11)

include(FetchContent)

FetchContent_Declare(
    argparse-cpp
    GIT_REPOSITORY https://github.com/your-username/argparse-cpp.git
    GIT_TAG main  # または特定のタグ・コミット
)

FetchContent_MakeAvailable(argparse-cpp)

# インクルードパスを追加
target_include_directories(your_target PRIVATE ${argparse-cpp_SOURCE_DIR}/include)
```

## 基本的な使い方

### 最小限の例

```cpp
#include "argparse/argparse.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    argparse::ArgumentParser parser("hello", "Hello world program");
    
    // 位置引数の追加
    parser.add_argument("name")
        .help("Your name");
    
    // オプション引数の追加
    parser.add_argument("--greeting", "-g")
        .default_value("Hello")
        .help("Greeting message");
    
    try {
        auto args = parser.parse_args(argc, argv);
        
        std::string name = args.get<std::string>("name");
        std::string greeting = args.get<std::string>("greeting");
        
        std::cout << greeting << ", " << name << "!" << std::endl;
        
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

### 使用例

```bash
# 基本的な使用
$ ./hello Alice
Hello, Alice!

# カスタムグリーティング
$ ./hello --greeting "Hi" Bob
Hi, Bob!

# ヘルプ表示
$ ./hello --help
usage: hello [-h] [--greeting GREETING] name

Hello world program

positional arguments:
  name                  Your name

options:
  -h, --help            show this help message and exit
  --greeting GREETING, -g GREETING
                        Greeting message
```

## 主要な機能

### 引数タイプ

#### 位置引数
```cpp
parser.add_argument("input_file")
    .help("Input file path");
```

#### オプション引数
```cpp
parser.add_argument("--output", "-o")
    .default_value("output.txt")
    .help("Output file path");
```

#### フラグ引数
```cpp
parser.add_argument("--verbose", "-v")
    .action("store_true")
    .help("Enable verbose output");
```

### 型変換

```cpp
// 整数引数
parser.add_argument("--count")
    .type<int>()
    .default_value(1)
    .help("Number of iterations");

// 浮動小数点引数
parser.add_argument("--rate")
    .type<float>()
    .default_value(1.0f)
    .help("Processing rate");

// 解析結果の取得
auto args = parser.parse_args(argc, argv);
int count = args.get<int>("count");
float rate = args.get<float>("rate");
```

### 選択肢の制限

```cpp
parser.add_argument("--mode")
    .choices({"fast", "normal", "slow"})
    .default_value("normal")
    .help("Processing mode");
```

### 引数の数制御

```cpp
// 0個以上
parser.add_argument("--files")
    .nargs("*")
    .help("Input files");

// 1個以上
parser.add_argument("--inputs")
    .nargs("+")
    .help("Required input files");

// 0個または1個
parser.add_argument("--config")
    .nargs("?")
    .default_value("default.conf")
    .help("Configuration file");
```

### 引数グループ

```cpp
auto* group = parser.add_argument_group("authentication", "Authentication options");
group->add_argument("--username")
    .required(true)
    .help("Username");
group->add_argument("--password")
    .required(true)
    .help("Password");
```

## API リファレンス

### ArgumentParser

#### コンストラクタ
```cpp
ArgumentParser(const std::string& prog, 
               const std::string& description = "",
               const std::string& epilog = "",
               bool add_help = true);
```

#### 主要メソッド
- `add_argument(name, ...)`: 引数を追加
- `add_argument_group(title, description)`: 引数グループを作成
- `parse_args(argc, argv)`: 引数を解析
- `parse_args(const std::vector<std::string>& args)`: 文字列ベクターから解析
- `format_help()`: ヘルプメッセージを生成

### Argument設定メソッド

- `.help(text)`: ヘルプテキストを設定
- `.type<T>()`: 型を指定（int, float, string, bool等）
- `.default_value(value)`: デフォルト値を設定
- `.required(bool)`: オプション引数を必須にする
- `.action(action)`: アクションを設定（"store", "store_true", "store_false"等）
- `.choices(values)`: 選択肢を制限
- `.nargs(count)`: 引数の数を指定
- `.metavar(name)`: ヘルプでの表示名を設定

### Namespace

解析結果を保持するクラス。

- `get<T>(name)`: 型安全な値取得
- `has(name)`: 引数が指定されたかチェック
- `contains(name)`: 引数が定義されているかチェック

## エラー処理

```cpp
try {
    auto args = parser.parse_args(argc, argv);
    // 正常処理
} catch (const std::exception& err) {
    std::cerr << "Error: " << err.what() << std::endl;
    return 1;
}
```

主要なエラータイプ：
- 必須引数の欠落
- 不正な型変換
- 不明なオプション
- 選択肢外の値
- 引数の数の不一致

## Python argparseとの違い

このライブラリはPython argparseのコア機能に焦点を当てており、以下の機能は現在サポートしていません：

- サブコマンド（subparsers）
- 相互排他グループ（mutually_exclusive_group）
- カスタムアクション
- ファイルタイプ（FileType）
- 設定ファイルからの引数読み込み

これらの機能は将来のバージョンで追加予定です。

## ライセンス

MIT License

## 貢献

Issue報告やプルリクエストを歓迎します。開発に参加される場合は、以下の点にご注意ください：

- C++11準拠のコードを維持
- Python argparseとの互換性を重視
- テストの追加
- ドキュメントの更新