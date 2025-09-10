#include <gtest/gtest.h>
#include "../../include/argparse/argparse.hpp"

class ParserIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // テストの前処理
    }
    
    void TearDown() override {
        // テストの後処理
    }
};

// ArgumentParserの基本的な初期化と引数追加の統合テスト
TEST_F(ParserIntegrationTest, BasicInitializationAndArguments) {
    argparse::ArgumentParser parser("integration_test", "Integration test program");
    
    // パーサーの基本プロパティを確認
    EXPECT_EQ(parser.prog(), "integration_test");
    EXPECT_EQ(parser.description(), "Integration test program");
    
    // 複数の引数を追加
    parser.add_argument("input_file").help("Input file path");
    parser.add_argument("--output", "-o").default_value(std::string("output.txt")).help("Output file path");
    parser.add_argument("--verbose", "-v").action("store_true").help("Enable verbose output");
    parser.add_argument("--count", "-c").type<int>().default_value(1).help("Number of iterations");
    
    // 引数が正しく追加されていることを確認
    EXPECT_EQ(parser.argument_count(), 5); // 4 + help option
    EXPECT_TRUE(parser.has_argument("input_file"));
    EXPECT_TRUE(parser.has_argument("--output"));
    EXPECT_TRUE(parser.has_argument("--verbose"));
    EXPECT_TRUE(parser.has_argument("--count"));
}

// コンポーネント連携：引数追加 -> 解析 -> 結果取得の統合テスト
TEST_F(ParserIntegrationTest, ArgumentAddingParsingAndRetrieval) {
    argparse::ArgumentParser parser("test_app", "Test application", "", false); // ヘルプ無効
    
    // 様々な種類の引数を追加
    parser.add_argument("command").help("Command to execute");
    parser.add_argument("--force", "-f").action("store_true").help("Force execution");
    parser.add_argument("--threads", "-t").type<int>().default_value(4).help("Number of threads");
    parser.add_argument("--config").default_value(std::string("config.json")).help("Configuration file");
    
    // 引数を解析
    std::vector<std::string> args = {"build", "--force", "--threads", "8", "--config", "custom.json"};
    auto namespace_result = parser.parse_args(args);
    
    // 解析結果を確認
    EXPECT_EQ(namespace_result.get<std::string>("command"), "build");
    EXPECT_EQ(namespace_result.get<bool>("force"), true);
    EXPECT_EQ(namespace_result.get<int>("threads"), 8);
    EXPECT_EQ(namespace_result.get<std::string>("config"), "custom.json");
}

// デフォルト値の統合テスト
TEST_F(ParserIntegrationTest, DefaultValueIntegration) {
    argparse::ArgumentParser parser("default_test");
    
    // デフォルト値を持つ引数を追加
    parser.add_argument("--timeout").type<int>().default_value(30);
    parser.add_argument("--mode").default_value(std::string("auto"));
    parser.add_argument("--enabled").action("store_true");
    parser.add_argument("--disabled").action("store_false");
    
    // 引数なしで解析
    std::vector<std::string> args = {};
    auto ns = parser.parse_args(args);
    
    // デフォルト値が正しく設定されていることを確認
    EXPECT_EQ(ns.get<int>("timeout"), 30);
    EXPECT_EQ(ns.get<std::string>("mode"), "auto");
    EXPECT_EQ(ns.get<bool>("enabled", false), false); // store_true の場合、デフォルトはfalse
    EXPECT_EQ(ns.get<bool>("disabled", true), true);  // store_false の場合、デフォルトはtrue
}

// 複雑な引数パターンの統合テスト
TEST_F(ParserIntegrationTest, ComplexArgumentPattern) {
    argparse::ArgumentParser parser("complex_app");
    
    // 位置引数
    parser.add_argument("source").help("Source directory");
    parser.add_argument("destination").help("Destination directory");
    
    // オプション引数（複数名前）
    parser.add_argument("--recursive", "-r").action("store_true").help("Recursive copy");
    parser.add_argument("--exclude", "-x").default_value(std::string("")).help("Exclude pattern");
    parser.add_argument("--max-size", "-s").type<int>().default_value(1024).help("Maximum file size");
    
    // 解析テスト1: 全引数指定
    std::vector<std::string> args1 = {"src/", "dst/", "-r", "--exclude", "*.tmp", "-s", "2048"};
    auto ns1 = parser.parse_args(args1);
    
    EXPECT_EQ(ns1.get<std::string>("source"), "src/");
    EXPECT_EQ(ns1.get<std::string>("destination"), "dst/");
    EXPECT_EQ(ns1.get<bool>("recursive"), true);
    EXPECT_EQ(ns1.get<std::string>("exclude"), "*.tmp");
    EXPECT_EQ(ns1.get<int>("max-size"), 2048);
    
    // 解析テスト2: 最小限の引数
    std::vector<std::string> args2 = {"input/", "output/"};
    auto ns2 = parser.parse_args(args2);
    
    EXPECT_EQ(ns2.get<std::string>("source"), "input/");
    EXPECT_EQ(ns2.get<std::string>("destination"), "output/");
    EXPECT_EQ(ns2.get<bool>("recursive", false), false);
    EXPECT_EQ(ns2.get<std::string>("exclude"), "");
    EXPECT_EQ(ns2.get<int>("max-size"), 1024);
}

// エラーハンドリングの統合テスト
TEST_F(ParserIntegrationTest, ErrorHandlingIntegration) {
    argparse::ArgumentParser parser("error_test", "", "", false); // ヘルプ無効
    
    parser.add_argument("required_arg").required(true);
    parser.add_argument("--optional").type<int>().default_value(0);
    
    // 必須引数が不足している場合
    std::vector<std::string> missing_args = {"--optional", "42"};
    EXPECT_THROW(parser.parse_args(missing_args), std::runtime_error);
    
    // 不明な引数が指定された場合
    std::vector<std::string> unknown_args = {"value", "--unknown-option"};
    EXPECT_THROW(parser.parse_args(unknown_args), std::runtime_error);
    
    // 型変換エラーの場合
    std::vector<std::string> type_error_args = {"value", "--optional", "not_a_number"};
    EXPECT_THROW(parser.parse_args(type_error_args), std::invalid_argument);
}

// argc/argv インターフェースの統合テスト
TEST_F(ParserIntegrationTest, ArgcArgvIntegration) {
    argparse::ArgumentParser parser("argc_argv_test");
    
    parser.add_argument("program_name").help("Program name");
    parser.add_argument("--debug", "-d").action("store_true").help("Debug mode");
    parser.add_argument("--level", "-l").type<int>().default_value(1).help("Log level");
    
    const char* argv[] = {"argc_argv_test", "my_program", "--debug", "-l", "3"};
    int argc = sizeof(argv) / sizeof(argv[0]);
    
    auto ns = parser.parse_args(argc, const_cast<char**>(argv));
    
    EXPECT_EQ(ns.get<std::string>("program_name"), "my_program");
    EXPECT_EQ(ns.get<bool>("debug"), true);
    EXPECT_EQ(ns.get<int>("level"), 3);
}

// AnyValueとTypeConverterの統合テスト
TEST_F(ParserIntegrationTest, AnyValueTypeConverterIntegration) {
    argparse::ArgumentParser parser("type_test");
    
    // 様々な型の引数を追加
    parser.add_argument("--str-val").default_value(std::string("default_string"));
    parser.add_argument("--int-val").type<int>().default_value(42);
    parser.add_argument("--bool-val").action("store_true");
    parser.add_argument("--double-val").type<double>().default_value(2.71828);
    
    std::vector<std::string> args = {
        "--str-val", "test_string",
        "--int-val", "100", 
        "--bool-val",
        "--double-val", "2.5"
    };
    
    auto ns = parser.parse_args(args);
    
    // 型変換が正しく行われていることを確認
    EXPECT_EQ(ns.get<std::string>("str-val"), "test_string");
    EXPECT_EQ(ns.get<int>("int-val"), 100);
    EXPECT_EQ(ns.get<bool>("bool-val"), true);
    EXPECT_DOUBLE_EQ(ns.get<double>("double-val"), 2.5);
}

// 引数グループとの統合テスト（ArgumentGroupが実装されている場合）
TEST_F(ParserIntegrationTest, ArgumentGroupIntegration) {
    argparse::ArgumentParser parser("group_test");
    
    // 基本引数
    parser.add_argument("input").help("Input file");
    
    // 出力オプショングループ
    auto& output_group = parser.add_argument_group("Output Options", "Options for controlling output");
    output_group.add_argument("--output", "-o").default_value(std::string("output.txt"));
    output_group.add_argument("--format", "-f").default_value(std::string("txt"));
    
    // ログオプショングループ  
    auto& log_group = parser.add_argument_group("Logging Options", "Options for controlling logging");
    log_group.add_argument("--verbose", "-v").action("store_true");
    log_group.add_argument("--quiet", "-q").action("store_true");
    
    std::vector<std::string> args = {
        "input.txt", 
        "--output", "result.json",
        "--format", "json", 
        "--verbose"
    };
    
    auto ns = parser.parse_args(args);
    
    EXPECT_EQ(ns.get<std::string>("input"), "input.txt");
    EXPECT_EQ(ns.get<std::string>("output"), "result.json");
    EXPECT_EQ(ns.get<std::string>("format"), "json");
    EXPECT_EQ(ns.get<bool>("verbose"), true);
    EXPECT_EQ(ns.get<bool>("quiet", false), false);
}

// 長時間実行や大量データの統合テスト
TEST_F(ParserIntegrationTest, LargeScaleIntegration) {
    argparse::ArgumentParser parser("large_test");
    
    // 大量の引数を追加（性能テスト）
    for (int i = 0; i < 50; ++i) {
        std::string arg_name = "--option" + std::to_string(i);
        parser.add_argument(arg_name).type<int>().default_value(i);
    }
    
    // 一部の引数を解析
    std::vector<std::string> args;
    for (int i = 0; i < 25; ++i) {
        args.push_back("--option" + std::to_string(i * 2));
        args.push_back(std::to_string(i * 100));
    }
    
    auto ns = parser.parse_args(args);
    
    // いくつかの値を確認
    EXPECT_EQ(ns.get<int>("option0"), 0);      // 指定された値
    EXPECT_EQ(ns.get<int>("option1"), 1);      // デフォルト値
    EXPECT_EQ(ns.get<int>("option2"), 100);    // 指定された値
    EXPECT_EQ(ns.get<int>("option3"), 3);      // デフォルト値
    EXPECT_EQ(ns.get<int>("option48"), 2400);  // 指定された値
    EXPECT_EQ(ns.get<int>("option49"), 49);    // デフォルト値
}

// Python argparse互換性の統合テスト
TEST_F(ParserIntegrationTest, PythonCompatibilityIntegration) {
    // Pythonのargparseと同じような使用パターンをテスト
    argparse::ArgumentParser parser("python_compat", 
                                  "Test Python argparse compatibility",
                                  "This is the epilog text");
    
    // 実際のAPIに合わせた引数定義
    parser.add_argument("filename")
        .help("Name of file to process");
    
    parser.add_argument("--verbose", "-v")
        .action("store_true")
        .help("Enable verbose output");
        
    parser.add_argument("--output", "-o")
        .default_value(std::string("output.txt"))
        .help("Output filename");
        
    parser.add_argument("--count", "-c")
        .type<int>()
        .default_value(1)
        .help("Number of times to process");
    
    // 解析
    std::vector<std::string> args = {"input.txt", "-v", "-o", "result.txt", "--count", "5"};
    auto ns = parser.parse_args(args);
    
    // 結果確認
    EXPECT_EQ(ns.get<std::string>("filename"), "input.txt");
    EXPECT_EQ(ns.get<bool>("verbose"), true);
    EXPECT_EQ(ns.get<std::string>("output"), "result.txt");
    EXPECT_EQ(ns.get<int>("count"), 5);
    
    // パーサー情報の確認
    EXPECT_EQ(parser.description(), "Test Python argparse compatibility");
    EXPECT_EQ(parser.epilog(), "This is the epilog text");
}

// メモリ管理とRAII統合テスト
TEST_F(ParserIntegrationTest, MemoryManagementRAII) {
    // スコープ内でのメモリ管理テスト
    {
        argparse::ArgumentParser parser("memory_test");
        parser.add_argument("--large-string").default_value(std::string(10000, 'x'));
        parser.add_argument("--numbers").type<int>().default_value(42);
        
        std::vector<std::string> args = {"--numbers", "999"};
        auto ns = parser.parse_args(args);
        
        EXPECT_EQ(ns.get<std::string>("large-string").size(), 10000);
        EXPECT_EQ(ns.get<int>("numbers"), 999);
        
        // AnyValueのコピーとムーブ
        auto ns_copy = ns;
        EXPECT_EQ(ns_copy.get<int>("numbers"), 999);
        
        auto ns_moved = std::move(ns);
        EXPECT_EQ(ns_moved.get<int>("numbers"), 999);
    } // すべてのオブジェクトが適切にデストラクトされる
    
    SUCCEED(); // メモリリークがなければ成功
}