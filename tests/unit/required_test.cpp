#include <gtest/gtest.h>
#include "../../include/argparse/argparse.hpp"
#include <stdexcept>

class RequiredTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// 必須オプション引数のテスト
TEST_F(RequiredTest, RequiredOptionArgument) {
    argparse::ArgumentParser parser;
    parser.add_argument("--input", "-i").required(true).help("Input file");
    parser.add_argument("--output", "-o").required(false).help("Output file");
    
    // 必須引数が指定された場合は成功
    {
        std::vector<std::string> args = {"--input", "test.txt"};
        auto ns = parser.parse_args(args);
        EXPECT_EQ(ns.get<std::string>("input"), "test.txt");
        EXPECT_FALSE(ns.has("output"));
    }
    
    // 両方指定された場合も成功
    {
        std::vector<std::string> args = {"--input", "test.txt", "--output", "result.txt"};
        auto ns = parser.parse_args(args);
        EXPECT_EQ(ns.get<std::string>("input"), "test.txt");
        EXPECT_EQ(ns.get<std::string>("output"), "result.txt");
    }
}

// 必須オプション引数が不足している場合のエラーテスト
TEST_F(RequiredTest, MissingRequiredOptionArgument) {
    argparse::ArgumentParser parser;
    parser.add_argument("--input", "-i").required(true).help("Input file");
    parser.add_argument("--output", "-o").required(false).help("Output file");
    
    // 必須引数が不足している場合はエラー
    {
        std::vector<std::string> args = {"--output", "result.txt"};
        EXPECT_THROW(parser.parse_args(args), std::runtime_error);
    }
    
    // 引数が全く指定されていない場合もエラー
    {
        std::vector<std::string> args = {};
        EXPECT_THROW(parser.parse_args(args), std::runtime_error);
    }
}

// 位置引数は常に必須であることの確認
TEST_F(RequiredTest, PositionalArgumentsAlwaysRequired) {
    argparse::ArgumentParser parser;
    parser.add_argument("filename").help("Input filename");
    
    // 位置引数が指定された場合は成功
    {
        std::vector<std::string> args = {"test.txt"};
        auto ns = parser.parse_args(args);
        EXPECT_EQ(ns.get<std::string>("filename"), "test.txt");
    }
    
    // 位置引数が不足している場合はエラー
    {
        std::vector<std::string> args = {};
        EXPECT_THROW(parser.parse_args(args), std::runtime_error);
    }
}

// デフォルト値がある場合のrequired設定テスト
TEST_F(RequiredTest, RequiredWithDefaultValue) {
    argparse::ArgumentParser parser;
    parser.add_argument("--config", "-c").required(true).default_value(std::string("default.conf")).help("Config file");
    
    // requiredがtrueでもデフォルト値があれば、引数が省略可能
    {
        std::vector<std::string> args = {};
        auto ns = parser.parse_args(args);
        EXPECT_EQ(ns.get<std::string>("config"), "default.conf");
    }
    
    // 引数が指定された場合はその値を使用
    {
        std::vector<std::string> args = {"--config", "custom.conf"};
        auto ns = parser.parse_args(args);
        EXPECT_EQ(ns.get<std::string>("config"), "custom.conf");
    }
}

// 複数の必須引数のテスト
TEST_F(RequiredTest, MultipleRequiredArguments) {
    argparse::ArgumentParser parser;
    parser.add_argument("--input", "-i").required(true).help("Input file");
    parser.add_argument("--output", "-o").required(true).help("Output file");
    parser.add_argument("--format", "-f").required(false).help("Output format");
    
    // 両方の必須引数が指定された場合は成功
    {
        std::vector<std::string> args = {"--input", "in.txt", "--output", "out.txt"};
        auto ns = parser.parse_args(args);
        EXPECT_EQ(ns.get<std::string>("input"), "in.txt");
        EXPECT_EQ(ns.get<std::string>("output"), "out.txt");
        EXPECT_FALSE(ns.has("format"));
    }
    
    // 一つでも必須引数が不足している場合はエラー
    {
        std::vector<std::string> args = {"--input", "in.txt"};
        EXPECT_THROW(parser.parse_args(args), std::runtime_error);
    }
    
    {
        std::vector<std::string> args = {"--output", "out.txt"};
        EXPECT_THROW(parser.parse_args(args), std::runtime_error);
    }
}

// store_true/store_falseアクションでのrequiredテスト
TEST_F(RequiredTest, RequiredBooleanFlags) {
    argparse::ArgumentParser parser;
    parser.add_argument("--verbose", "-v").action("store_true").required(true).help("Verbose output");
    parser.add_argument("--quiet", "-q").action("store_true").required(false).help("Quiet output");
    
    // 必須のstore_trueフラグが指定された場合は成功
    {
        std::vector<std::string> args = {"--verbose"};
        auto ns = parser.parse_args(args);
        EXPECT_TRUE(ns.get<bool>("verbose"));
        EXPECT_TRUE(ns.has("quiet"));        // quietには false が設定されるのでhas()はtrue
        EXPECT_FALSE(ns.get<bool>("quiet")); // quietの値はfalse
    }
    
    // 必須のstore_trueフラグが不足している場合はエラー
    {
        std::vector<std::string> args = {"--quiet"};
        EXPECT_THROW(parser.parse_args(args), std::runtime_error);
    }
    
    {
        std::vector<std::string> args = {};
        EXPECT_THROW(parser.parse_args(args), std::runtime_error);
    }
}

// エラーメッセージの内容確認
TEST_F(RequiredTest, RequiredArgumentErrorMessage) {
    argparse::ArgumentParser parser;
    parser.add_argument("--input", "-i").required(true).help("Input file");
    
    std::vector<std::string> args = {};
    try {
        parser.parse_args(args);
        FAIL() << "Expected std::runtime_error to be thrown";
    } catch (const std::runtime_error& e) {
        std::string error_msg = e.what();
        // エラーメッセージに必須引数の名前が含まれていることを確認
        EXPECT_TRUE(error_msg.find("--input") != std::string::npos || 
                    error_msg.find("input") != std::string::npos);
        EXPECT_TRUE(error_msg.find("Required") != std::string::npos || 
                    error_msg.find("required") != std::string::npos ||
                    error_msg.find("missing") != std::string::npos);
    }
}