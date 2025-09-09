#include <gtest/gtest.h>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include "argparse/argparse.hpp"

using namespace argparse;
using namespace argparse::detail;

class ErrorMessageTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Redirect cerr to capture error messages
        original_cerr_buf = std::cerr.rdbuf();
        std::cerr.rdbuf(cerr_stream.rdbuf());
        
        // Redirect cout to capture help messages
        original_cout_buf = std::cout.rdbuf();
        std::cout.rdbuf(cout_stream.rdbuf());
    }
    
    void TearDown() override {
        // Restore original cerr and cout
        std::cerr.rdbuf(original_cerr_buf);
        std::cout.rdbuf(original_cout_buf);
    }
    
    std::ostringstream cerr_stream;
    std::ostringstream cout_stream;
    std::streambuf* original_cerr_buf;
    std::streambuf* original_cout_buf;
    
    // Helper function to capture parse_args exceptions
    std::string capture_error_message(ArgumentParser& parser, const std::vector<std::string>& args) {
        cerr_stream.str("");
        cerr_stream.clear();
        
        // ArgumentParser::parse_args calls exit(2) on error, so we need to test the underlying parser
        detail::Parser internal_parser;
        try {
            internal_parser.parse(args, parser.get_arguments());
            return "";  // No error occurred
        } catch (const ArgumentError& e) {
            return ErrorFormatter::format_error_with_usage(parser, e.what());
        } catch (const std::invalid_argument& e) {
            return ErrorFormatter::format_error_with_usage(parser, e.what());
        } catch (...) {
            return "Unknown error";
        }
    }
};

// ArgumentError例外クラスのテスト
TEST_F(ErrorMessageTest, ArgumentErrorBasicFunctionality) {
    // Constructor with argument name and message
    ArgumentError error("--count", "invalid int value: 'abc'");
    std::string expected = "argument --count: invalid int value: 'abc'";
    EXPECT_EQ(std::string(error.what()), expected);
    EXPECT_EQ(error.argument_name(), "--count");
    EXPECT_EQ(error.error_message(), "invalid int value: 'abc'");
    
    // Constructor with message only
    ArgumentError general_error("unrecognized arguments: --unknown");
    EXPECT_EQ(std::string(general_error.what()), "unrecognized arguments: --unknown");
    EXPECT_EQ(general_error.argument_name(), "");
    EXPECT_EQ(general_error.error_message(), "unrecognized arguments: --unknown");
}

// ErrorFormatter::format_error テスト
TEST_F(ErrorMessageTest, ErrorFormatterBasicError) {
    ArgumentParser parser("testprog");
    std::string error_msg = ErrorFormatter::format_error(parser, "invalid argument");
    std::string expected = "testprog: error: invalid argument\n";
    EXPECT_EQ(error_msg, expected);
}

// ErrorFormatter::format_error_with_usage テスト
TEST_F(ErrorMessageTest, ErrorFormatterWithUsage) {
    ArgumentParser parser("testprog", "Test program");
    parser.add_argument("input").help("Input file");
    parser.add_argument("--verbose", "-v").action("store_true").help("Verbose output");
    
    std::string error_msg = ErrorFormatter::format_error_with_usage(parser, "invalid argument");
    
    // Expected format: usage line + error line
    EXPECT_TRUE(error_msg.find("usage: testprog") == 0);
    EXPECT_TRUE(error_msg.find("testprog: error: invalid argument") != std::string::npos);
    EXPECT_TRUE(error_msg.find("[options]") != std::string::npos);
    EXPECT_TRUE(error_msg.find("input") != std::string::npos);
}

// 引数固有のエラーメッセージフォーマット
TEST_F(ErrorMessageTest, ArgumentSpecificErrorMessages) {
    // Required argument error
    std::string error = ErrorFormatter::format_argument_error("--input", "required");
    EXPECT_EQ(error, "the following arguments are required: --input");
    
    // Unknown argument error
    error = ErrorFormatter::format_argument_error("--unknown", "unknown");
    EXPECT_EQ(error, "unrecognized arguments: --unknown");
    
    // Type conversion error
    error = ErrorFormatter::format_argument_error("--count", "type_conversion", "int value: 'abc'");
    EXPECT_EQ(error, "argument --count: invalid int value: 'abc'");
    
    // Missing value error
    error = ErrorFormatter::format_argument_error("--output", "missing_value");
    EXPECT_EQ(error, "argument --output: expected one argument");
}

// Choices エラーメッセージ
TEST_F(ErrorMessageTest, ChoicesErrorMessage) {
    std::vector<std::string> choices = {"apple", "banana", "orange"};
    std::string error = ErrorFormatter::format_choices_error("--fruit", "grape", choices);
    std::string expected = "argument --fruit: invalid choice: 'grape' (choose from 'apple', 'banana', 'orange')";
    EXPECT_EQ(error, expected);
}

// Type conversion エラーメッセージ
TEST_F(ErrorMessageTest, TypeConversionErrorMessage) {
    std::string error = ErrorFormatter::format_type_error("--count", "abc", "int");
    EXPECT_EQ(error, "argument --count: invalid int value: 'abc'");
    
    error = ErrorFormatter::format_type_error("--ratio", "xyz", "float");
    EXPECT_EQ(error, "argument --ratio: invalid float value: 'xyz'");
}

// 複数の必須引数のエラー
TEST_F(ErrorMessageTest, MultipleRequiredArgumentsError) {
    std::vector<std::string> missing_args = {"--input", "--output"};
    std::string error = ErrorFormatter::format_multiple_required_error(missing_args);
    EXPECT_EQ(error, "the following arguments are required: --input, --output");
    
    // Empty case
    std::vector<std::string> empty_args;
    error = ErrorFormatter::format_multiple_required_error(empty_args);
    EXPECT_EQ(error, "required arguments are missing");
}

// 実際の解析エラーのテスト
TEST_F(ErrorMessageTest, UnknownArgumentError) {
    ArgumentParser parser("testprog");
    parser.add_argument("input");
    
    std::string error_msg = capture_error_message(parser, {"--unknown"});
    
    EXPECT_TRUE(error_msg.find("usage: testprog") == 0);
    EXPECT_TRUE(error_msg.find("unrecognized arguments: --unknown") != std::string::npos);
}

TEST_F(ErrorMessageTest, RequiredArgumentError) {
    ArgumentParser parser("testprog");
    parser.add_argument("--input").required(true);
    parser.add_argument("--output").required(true);
    
    std::string error_msg = capture_error_message(parser, {});
    
    EXPECT_TRUE(error_msg.find("usage: testprog") == 0);
    EXPECT_TRUE(error_msg.find("the following arguments are required:") != std::string::npos);
    EXPECT_TRUE(error_msg.find("--input") != std::string::npos);
    EXPECT_TRUE(error_msg.find("--output") != std::string::npos);
}

TEST_F(ErrorMessageTest, PositionalArgumentError) {
    ArgumentParser parser("testprog");
    parser.add_argument("input");
    
    std::string error_msg = capture_error_message(parser, {});
    
    EXPECT_TRUE(error_msg.find("usage: testprog") == 0);
    EXPECT_TRUE(error_msg.find("the following arguments are required:") != std::string::npos);
    EXPECT_TRUE(error_msg.find("input") != std::string::npos);
}

TEST_F(ErrorMessageTest, TypeConversionError) {
    ArgumentParser parser("testprog");
    parser.add_argument("--count").type("int");
    
    std::string error_msg = capture_error_message(parser, {"--count", "abc"});
    
    EXPECT_TRUE(error_msg.find("usage: testprog") == 0);
    EXPECT_TRUE(error_msg.find("invalid int value") != std::string::npos);
    EXPECT_TRUE(error_msg.find("'abc'") != std::string::npos);
}

TEST_F(ErrorMessageTest, ChoicesValidationError) {
    ArgumentParser parser("testprog");
    parser.add_argument("--fruit").choices(std::vector<std::string>{"apple", "banana", "orange"});
    
    std::string error_msg = capture_error_message(parser, {"--fruit", "grape"});
    
    EXPECT_TRUE(error_msg.find("usage: testprog") == 0);
    EXPECT_TRUE(error_msg.find("invalid choice") != std::string::npos);
    EXPECT_TRUE(error_msg.find("'grape'") != std::string::npos);
    EXPECT_TRUE(error_msg.find("choose from") != std::string::npos);
    EXPECT_TRUE(error_msg.find("'apple'") != std::string::npos);
}

TEST_F(ErrorMessageTest, TooManyPositionalArgumentsError) {
    ArgumentParser parser("testprog");
    parser.add_argument("input");
    
    std::string error_msg = capture_error_message(parser, {"file1.txt", "file2.txt"});
    
    EXPECT_TRUE(error_msg.find("usage: testprog") == 0);
    EXPECT_TRUE(error_msg.find("unrecognized arguments: file2.txt") != std::string::npos);
}

// Usage行の生成テスト
TEST_F(ErrorMessageTest, UsageLineGeneration) {
    ArgumentParser parser("myprogram");
    parser.add_argument("input").help("Input file");
    parser.add_argument("--verbose", "-v").action("store_true");
    parser.add_argument("--count").type("int").default_value(1);
    parser.add_argument("--required-opt").required(true);
    
    std::string error_msg = ErrorFormatter::format_error_with_usage(parser, "test error");
    
    // Check usage line contains expected elements
    EXPECT_TRUE(error_msg.find("usage: myprogram") == 0);
    EXPECT_TRUE(error_msg.find("[options]") != std::string::npos);
    EXPECT_TRUE(error_msg.find("--required-opt") != std::string::npos);
    EXPECT_TRUE(error_msg.find("input") != std::string::npos);
    EXPECT_TRUE(error_msg.find("myprogram: error: test error") != std::string::npos);
}

// エラーメッセージのフォーマット一貫性テスト
TEST_F(ErrorMessageTest, ErrorMessageFormatConsistency) {
    // Python argparseと同様のフォーマットかテスト
    ArgumentParser parser("prog");
    
    // 1. Program name should appear in error
    std::string error = ErrorFormatter::format_error(parser, "test");
    EXPECT_TRUE(error.find("prog: error:") != std::string::npos);
    
    // 2. Usage should start with "usage: prog"
    error = ErrorFormatter::format_error_with_usage(parser, "test");
    EXPECT_TRUE(error.find("usage: prog") == 0);
    
    // 3. Error line format
    EXPECT_TRUE(error.find("prog: error: test") != std::string::npos);
    EXPECT_TRUE(error.back() == '\n');
}