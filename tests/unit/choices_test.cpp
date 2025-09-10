#include <gtest/gtest.h>
#include "../../include/argparse/argparse.hpp"

using namespace argparse;

class ChoicesTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    
    void TearDown() override {
    }
};

// 文字列の選択肢設定のテスト
TEST_F(ChoicesTest, StringChoices) {
    Argument arg("--mode");
    arg.choices(std::vector<std::string>{"debug", "info", "warning", "error"});
    
    // 有効な値の検証
    detail::AnyValue valid_value(std::string("debug"));
    EXPECT_TRUE(arg.validate_value(valid_value));
    
    detail::AnyValue valid_value2(std::string("error"));
    EXPECT_TRUE(arg.validate_value(valid_value2));
    
    // 無効な値の検証
    detail::AnyValue invalid_value(std::string("invalid"));
    EXPECT_FALSE(arg.validate_value(invalid_value));
}

// 整数の選択肢設定のテスト
TEST_F(ChoicesTest, IntChoices) {
    Argument arg("--count");
    arg.choices(std::vector<int>{1, 2, 4, 8, 16});
    
    // 有効な値の検証
    detail::AnyValue valid_value(1);
    EXPECT_TRUE(arg.validate_value(valid_value));
    
    detail::AnyValue valid_value2(16);
    EXPECT_TRUE(arg.validate_value(valid_value2));
    
    // 無効な値の検証
    detail::AnyValue invalid_value(3);
    EXPECT_FALSE(arg.validate_value(invalid_value));
    
    detail::AnyValue invalid_value2(0);
    EXPECT_FALSE(arg.validate_value(invalid_value2));
}

// エラーメッセージのテスト
TEST_F(ChoicesTest, ErrorMessage) {
    Argument arg("--mode");
    arg.choices(std::vector<std::string>{"debug", "info", "warning"});
    
    detail::AnyValue invalid_value(std::string("invalid"));
    EXPECT_FALSE(arg.validate_value(invalid_value));
    
    std::string error_msg = arg.get_validation_error_message(invalid_value);
    
    // エラーメッセージに無効な値と選択肢が含まれているかをチェック
    EXPECT_NE(error_msg.find("invalid choice"), std::string::npos);
    EXPECT_NE(error_msg.find("'invalid'"), std::string::npos);
    EXPECT_NE(error_msg.find("'debug'"), std::string::npos);
    EXPECT_NE(error_msg.find("'info'"), std::string::npos);
    EXPECT_NE(error_msg.find("'warning'"), std::string::npos);
}

// ArgumentParserでのchoices統合テスト
TEST_F(ChoicesTest, ArgumentParserIntegration) {
    ArgumentParser parser("test_program");
    
    parser.add_argument("--mode")
        .choices(std::vector<std::string>{"debug", "info", "warning", "error"})
        .help("Log level mode");
    
    // 有効な値での解析
    char* args_valid[] = {const_cast<char*>("test_program"), const_cast<char*>("--mode"), const_cast<char*>("debug")};
    auto result_valid = parser.parse_args(3, args_valid);
    EXPECT_EQ(result_valid.get<std::string>("mode"), "debug");
    
    // 無効な値での解析（例外が発生することを確認）
    char* args_invalid[] = {const_cast<char*>("test_program"), const_cast<char*>("--mode"), const_cast<char*>("invalid")};
    EXPECT_THROW({
        parser.parse_args(3, args_invalid);
    }, std::invalid_argument);
}

// 位置引数でのchoicesテスト
TEST_F(ChoicesTest, PositionalArgumentChoices) {
    ArgumentParser parser("test_program");
    
    parser.add_argument("action")
        .choices(std::vector<std::string>{"start", "stop", "restart", "status"})
        .help("Action to perform");
    
    // 有効な値での解析
    char* args_valid[] = {const_cast<char*>("test_program"), const_cast<char*>("start")};
    auto result_valid = parser.parse_args(2, args_valid);
    EXPECT_EQ(result_valid.get<std::string>("action"), "start");
    
    // 無効な値での解析（例外が発生することを確認）
    char* args_invalid[] = {const_cast<char*>("test_program"), const_cast<char*>("invalid_action")};
    EXPECT_THROW({
        parser.parse_args(2, args_invalid);
    }, std::invalid_argument);
}

// 型変換とchoicesの組み合わせテスト
TEST_F(ChoicesTest, TypeConversionWithChoices) {
    ArgumentParser parser("test_program");
    
    parser.add_argument("--port")
        .type<int>()
        .choices(std::vector<int>{80, 443, 8080, 8443})
        .help("Port number");
    
    // 有効な値での解析
    char* args_valid[] = {const_cast<char*>("test_program"), const_cast<char*>("--port"), const_cast<char*>("80")};
    auto result_valid = parser.parse_args(3, args_valid);
    EXPECT_EQ(result_valid.get<int>("port"), 80);
    
    // 無効な値での解析
    char* args_invalid[] = {const_cast<char*>("test_program"), const_cast<char*>("--port"), const_cast<char*>("22")};
    EXPECT_THROW({
        parser.parse_args(3, args_invalid);
    }, std::invalid_argument);
}

// エラーメッセージの詳細確認
TEST_F(ChoicesTest, DetailedErrorMessage) {
    ArgumentParser parser("test_program");
    
    parser.add_argument("--level")
        .choices(std::vector<std::string>{"low", "medium", "high"})
        .help("Priority level");
    
    char* args[] = {const_cast<char*>("test_program"), const_cast<char*>("--level"), const_cast<char*>("extreme")};
    
    try {
        parser.parse_args(3, args);
        FAIL() << "Expected std::invalid_argument to be thrown";
    } catch (const std::invalid_argument& e) {
        std::string error_msg(e.what());
        
        // エラーメッセージの形式を確認
        EXPECT_NE(error_msg.find("argument --level"), std::string::npos);
        EXPECT_NE(error_msg.find("invalid choice"), std::string::npos);
        EXPECT_NE(error_msg.find("'extreme'"), std::string::npos);
        EXPECT_NE(error_msg.find("choose from"), std::string::npos);
        EXPECT_NE(error_msg.find("'low'"), std::string::npos);
        EXPECT_NE(error_msg.find("'medium'"), std::string::npos);
        EXPECT_NE(error_msg.find("'high'"), std::string::npos);
    }
}