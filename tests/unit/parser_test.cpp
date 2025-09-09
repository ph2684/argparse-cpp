#include <gtest/gtest.h>
#include "../../include/argparse/argparse.hpp"

class ParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        parser = std::make_unique<argparse::ArgumentParser>("test_parser", "Parser test program");
    }

    void TearDown() override {
        parser.reset();
    }

    std::unique_ptr<argparse::ArgumentParser> parser;
};

// トークンと引数のマッチングテスト
TEST_F(ParserTest, TokenArgumentMatching) {
    // 位置引数の設定
    parser->add_argument("filename").help("Input filename");
    
    // オプション引数の設定
    parser->add_argument("-v", "--verbose").action("store_true").help("Enable verbose output");
    parser->add_argument("-n", "--number").type("int").default_value(42).help("A number");
    
    // 基本的なマッチングテスト
    std::vector<std::string> args = {"input.txt", "--verbose", "--number", "123"};
    auto result = parser->parse_args(args);
    
    EXPECT_EQ(result.get<std::string>("filename"), "input.txt");
    EXPECT_TRUE(result.get<bool>("verbose"));
    EXPECT_EQ(result.get<int>("number"), 123);
}

// 位置引数の順序処理テスト
TEST_F(ParserTest, PositionalArgumentOrder) {
    // 複数の位置引数を定義
    parser->add_argument("first").help("First argument");
    parser->add_argument("second").help("Second argument");
    parser->add_argument("third").help("Third argument");
    
    std::vector<std::string> args = {"arg1", "arg2", "arg3"};
    auto result = parser->parse_args(args);
    
    EXPECT_EQ(result.get<std::string>("first"), "arg1");
    EXPECT_EQ(result.get<std::string>("second"), "arg2");
    EXPECT_EQ(result.get<std::string>("third"), "arg3");
}

// オプション引数の検索テスト
TEST_F(ParserTest, OptionArgumentSearch) {
    // 短縮形と長形式の両方を定義
    parser->add_argument("-v", "--verbose").action("store_true").help("Verbose mode");
    parser->add_argument("-o", "--output").type("string").help("Output file");
    
    // 短縮形でのテスト
    std::vector<std::string> args1 = {"-v", "-o", "out.txt"};
    auto result1 = parser->parse_args(args1);
    
    EXPECT_TRUE(result1.get<bool>("verbose"));
    EXPECT_EQ(result1.get<std::string>("output"), "out.txt");
    
    // 長形式でのテスト
    std::vector<std::string> args2 = {"--verbose", "--output", "output.log"};
    auto result2 = parser->parse_args(args2);
    
    EXPECT_TRUE(result2.get<bool>("verbose"));
    EXPECT_EQ(result2.get<std::string>("output"), "output.log");
}

// 不明な引数のエラーテスト
TEST_F(ParserTest, UnknownArgumentError) {
    parser->add_argument("filename").help("Input filename");
    parser->add_argument("-v", "--verbose").action("store_true").help("Verbose mode");
    
    // 不明なオプション
    std::vector<std::string> args1 = {"input.txt", "--unknown"};
    EXPECT_THROW(parser->parse_args(args1), std::runtime_error);
    
    // 不明な短縮形オプション
    std::vector<std::string> args2 = {"input.txt", "-x"};
    EXPECT_THROW(parser->parse_args(args2), std::runtime_error);
}

// デフォルト値の設定テスト
TEST_F(ParserTest, DefaultValues) {
    parser->add_argument("filename").help("Input filename");
    parser->add_argument("-n", "--number").type("int").default_value(42).help("A number");
    parser->add_argument("-s", "--string").type("string").default_value(std::string("default")).help("A string");
    
    // デフォルト値のみを使用
    std::vector<std::string> args = {"input.txt"};
    auto result = parser->parse_args(args);
    
    EXPECT_EQ(result.get<std::string>("filename"), "input.txt");
    EXPECT_EQ(result.get<int>("number"), 42);
    EXPECT_EQ(result.get<std::string>("string"), "default");
}

// 型変換テスト
TEST_F(ParserTest, TypeConversion) {
    parser->add_argument("-i", "--int").type("int").help("Integer value");
    parser->add_argument("-f", "--float").type("float").help("Float value");
    parser->add_argument("-b", "--bool").type("bool").help("Boolean value");
    
    std::vector<std::string> args = {"--int", "123", "--float", "3.14", "--bool", "true"};
    auto result = parser->parse_args(args);
    
    EXPECT_EQ(result.get<int>("int"), 123);
    EXPECT_DOUBLE_EQ(result.get<double>("float"), 3.14);
    EXPECT_TRUE(result.get<bool>("bool"));
}

// store_true/store_false アクションテスト
TEST_F(ParserTest, BooleanActions) {
    parser->add_argument("-v", "--verbose").action("store_true").help("Verbose mode");
    parser->add_argument("-q", "--quiet").action("store_false").help("Quiet mode");
    
    // store_true テスト
    std::vector<std::string> args1 = {"--verbose"};
    auto result1 = parser->parse_args(args1);
    EXPECT_TRUE(result1.get<bool>("verbose"));
    
    // store_false テスト
    std::vector<std::string> args2 = {"--quiet"};
    auto result2 = parser->parse_args(args2);
    EXPECT_FALSE(result2.get<bool>("quiet"));
}

// 引数が多すぎる場合のエラーテスト
TEST_F(ParserTest, TooManyPositionalArguments) {
    parser->add_argument("filename").help("Input filename");
    
    std::vector<std::string> args = {"input.txt", "extra_arg"};
    EXPECT_THROW(parser->parse_args(args), std::runtime_error);
}

// 必須引数のテスト（今後のために準備）
TEST_F(ParserTest, RequiredArguments) {
    parser->add_argument("filename").help("Input filename");
    parser->add_argument("-r", "--required").required(true).help("Required option");
    
    // 必須引数が不足している場合
    std::vector<std::string> args1 = {"input.txt"};
    EXPECT_THROW(parser->parse_args(args1), std::runtime_error);
    
    // 必須引数が提供されている場合
    std::vector<std::string> args2 = {"input.txt", "--required", "value"};
    auto result = parser->parse_args(args2);
    EXPECT_EQ(result.get<std::string>("filename"), "input.txt");
    EXPECT_EQ(result.get<std::string>("required"), "value");
}

// オプション値が不足している場合のエラーテスト
TEST_F(ParserTest, MissingOptionValue) {
    parser->add_argument("-o", "--output").type("string").help("Output file");
    
    std::vector<std::string> args = {"--output"};
    EXPECT_THROW(parser->parse_args(args), std::runtime_error);
}

// 混合引数テスト（位置引数とオプション引数の組み合わせ）
TEST_F(ParserTest, MixedArguments) {
    parser->add_argument("input").help("Input file");
    parser->add_argument("output").help("Output file");
    parser->add_argument("-v", "--verbose").action("store_true").help("Verbose mode");
    parser->add_argument("-c", "--count").type("int").default_value(1).help("Count");
    
    std::vector<std::string> args = {"in.txt", "out.txt", "--verbose", "--count", "5"};
    auto result = parser->parse_args(args);
    
    EXPECT_EQ(result.get<std::string>("input"), "in.txt");
    EXPECT_EQ(result.get<std::string>("output"), "out.txt");
    EXPECT_TRUE(result.get<bool>("verbose"));
    EXPECT_EQ(result.get<int>("count"), 5);
}

// 型変換エラーテスト
TEST_F(ParserTest, TypeConversionError) {
    parser->add_argument("-n", "--number").type("int").help("Number");
    
    std::vector<std::string> args = {"--number", "not_a_number"};
    EXPECT_THROW(parser->parse_args(args), std::invalid_argument);
}

// エンドオプションマーカー（--）のテスト
TEST_F(ParserTest, EndOptionsMarker) {
    parser->add_argument("files").help("Input files");
    parser->add_argument("-v", "--verbose").action("store_true").help("Verbose mode");
    
    // -- 以降はすべて位置引数として扱われる
    std::vector<std::string> args = {"--verbose", "--", "--file-that-looks-like-option"};
    auto result = parser->parse_args(args);
    
    EXPECT_TRUE(result.get<bool>("verbose"));
    EXPECT_EQ(result.get<std::string>("files"), "--file-that-looks-like-option");
}