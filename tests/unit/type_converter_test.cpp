#include <gtest/gtest.h>
#include <cmath>
#include "../../include/argparse/argparse.hpp"

using namespace argparse::detail;

class TypeConverterTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    
    void TearDown() override {
    }
};

// Int変換のテスト
TEST_F(TypeConverterTest, IntConverter) {
    auto converter = TypeConverter::int_converter();
    
    // 正常な変換
    AnyValue result = converter("42");
    EXPECT_EQ(result.get<int>(), 42);
    
    result = converter("-123");
    EXPECT_EQ(result.get<int>(), -123);
    
    result = converter("0");
    EXPECT_EQ(result.get<int>(), 0);
    
    // 先頭・末尾の空白は無視される
    result = converter("  42  ");
    EXPECT_EQ(result.get<int>(), 42);
    
    result = converter("\t123\n");
    EXPECT_EQ(result.get<int>(), 123);
}

TEST_F(TypeConverterTest, IntConverterThrowsOnInvalidInput) {
    auto converter = TypeConverter::int_converter();
    
    // 不正な入力
    EXPECT_THROW(converter(""), std::invalid_argument);
    EXPECT_THROW(converter("abc"), std::invalid_argument);
    EXPECT_THROW(converter("12.5"), std::invalid_argument);
    EXPECT_THROW(converter("123abc"), std::invalid_argument);
    EXPECT_THROW(converter("abc123"), std::invalid_argument);
    EXPECT_THROW(converter("   "), std::invalid_argument);
}

TEST_F(TypeConverterTest, IntConverterThrowsOnOutOfRange) {
    auto converter = TypeConverter::int_converter();
    
    // 範囲外の値（実際の範囲はプラットフォーム依存）
    EXPECT_THROW(converter("999999999999999999999"), std::invalid_argument);
    EXPECT_THROW(converter("-999999999999999999999"), std::invalid_argument);
}

// Float変換のテスト
TEST_F(TypeConverterTest, FloatConverter) {
    auto converter = TypeConverter::float_converter();
    
    // 正常な変換
    AnyValue result = converter("3.14159");
    EXPECT_DOUBLE_EQ(result.get<double>(), 3.14159);
    
    result = converter("-2.5");
    EXPECT_DOUBLE_EQ(result.get<double>(), -2.5);
    
    result = converter("0.0");
    EXPECT_DOUBLE_EQ(result.get<double>(), 0.0);
    
    result = converter("42");  // 整数も変換可能
    EXPECT_DOUBLE_EQ(result.get<double>(), 42.0);
    
    // 科学記法
    result = converter("1.5e-10");
    EXPECT_DOUBLE_EQ(result.get<double>(), 1.5e-10);
    
    result = converter("2E+5");
    EXPECT_DOUBLE_EQ(result.get<double>(), 2E+5);
    
    // 先頭・末尾の空白は無視される
    result = converter("  3.14  ");
    EXPECT_DOUBLE_EQ(result.get<double>(), 3.14);
}

TEST_F(TypeConverterTest, FloatConverterThrowsOnInvalidInput) {
    auto converter = TypeConverter::float_converter();
    
    // 不正な入力
    EXPECT_THROW(converter(""), std::invalid_argument);
    EXPECT_THROW(converter("abc"), std::invalid_argument);
    EXPECT_THROW(converter("3.14abc"), std::invalid_argument);
    EXPECT_THROW(converter("abc3.14"), std::invalid_argument);
    EXPECT_THROW(converter("   "), std::invalid_argument);
}

TEST_F(TypeConverterTest, FloatConverterHandlesInfinityAndNaN) {
    auto converter = TypeConverter::float_converter();
    
    // inf, -inf, nanは標準ライブラリの実装による
    // 一般的にはサポートされるが、プラットフォーム依存
    AnyValue result;
    
    // infが変換できるかテスト（エラーでも正常でもOK）
    try {
        result = converter("inf");
        EXPECT_TRUE(std::isinf(result.get<double>()));
    } catch (const std::invalid_argument&) {
        // infをサポートしない実装もある
    }
    
    try {
        result = converter("-inf");
        EXPECT_TRUE(std::isinf(result.get<double>()));
        EXPECT_LT(result.get<double>(), 0);
    } catch (const std::invalid_argument&) {
        // -infをサポートしない実装もある
    }
}

// Bool変換のテスト
TEST_F(TypeConverterTest, BoolConverter) {
    auto converter = TypeConverter::bool_converter();
    
    // true値
    EXPECT_TRUE(converter("true").get<bool>());
    EXPECT_TRUE(converter("TRUE").get<bool>());
    EXPECT_TRUE(converter("True").get<bool>());
    EXPECT_TRUE(converter("1").get<bool>());
    EXPECT_TRUE(converter("yes").get<bool>());
    EXPECT_TRUE(converter("YES").get<bool>());
    EXPECT_TRUE(converter("on").get<bool>());
    EXPECT_TRUE(converter("ON").get<bool>());
    
    // false値
    EXPECT_FALSE(converter("false").get<bool>());
    EXPECT_FALSE(converter("FALSE").get<bool>());
    EXPECT_FALSE(converter("False").get<bool>());
    EXPECT_FALSE(converter("0").get<bool>());
    EXPECT_FALSE(converter("no").get<bool>());
    EXPECT_FALSE(converter("NO").get<bool>());
    EXPECT_FALSE(converter("off").get<bool>());
    EXPECT_FALSE(converter("OFF").get<bool>());
    
    // 空白の処理
    EXPECT_TRUE(converter("  true  ").get<bool>());
    EXPECT_FALSE(converter("\tfalse\n").get<bool>());
}

TEST_F(TypeConverterTest, BoolConverterThrowsOnInvalidInput) {
    auto converter = TypeConverter::bool_converter();
    
    EXPECT_THROW(converter(""), std::invalid_argument);
    EXPECT_THROW(converter("maybe"), std::invalid_argument);
    EXPECT_THROW(converter("2"), std::invalid_argument);
    EXPECT_THROW(converter("truee"), std::invalid_argument);
    EXPECT_THROW(converter("   "), std::invalid_argument);
}

// String変換のテスト
TEST_F(TypeConverterTest, StringConverter) {
    auto converter = TypeConverter::string_converter();
    
    // 任意の文字列が変換される（そのまま返される）
    EXPECT_EQ(converter("hello").get<std::string>(), "hello");
    EXPECT_EQ(converter("").get<std::string>(), "");
    EXPECT_EQ(converter("123").get<std::string>(), "123");
    EXPECT_EQ(converter("   spaces   ").get<std::string>(), "   spaces   ");
    EXPECT_EQ(converter("special!@#$%^&*()").get<std::string>(), "special!@#$%^&*()");
}

// get_converter_by_name のテスト
TEST_F(TypeConverterTest, GetConverterByName) {
    // int
    auto int_conv = TypeConverter::get_converter_by_name("int");
    EXPECT_EQ(int_conv("42").get<int>(), 42);
    
    // float
    auto float_conv = TypeConverter::get_converter_by_name("float");
    EXPECT_DOUBLE_EQ(float_conv("3.14").get<double>(), 3.14);
    
    // double
    auto double_conv = TypeConverter::get_converter_by_name("double");
    EXPECT_DOUBLE_EQ(double_conv("2.71").get<double>(), 2.71);
    
    // bool
    auto bool_conv = TypeConverter::get_converter_by_name("bool");
    EXPECT_TRUE(bool_conv("true").get<bool>());
    
    // string
    auto str_conv = TypeConverter::get_converter_by_name("string");
    EXPECT_EQ(str_conv("test").get<std::string>(), "test");
    
    // str
    auto str_conv2 = TypeConverter::get_converter_by_name("str");
    EXPECT_EQ(str_conv2("test").get<std::string>(), "test");
    
    // 未知の型名 -> デフォルトでstring変換
    auto unknown_conv = TypeConverter::get_converter_by_name("unknown_type");
    EXPECT_EQ(unknown_conv("test").get<std::string>(), "test");
}

// テンプレート特殊化のテスト
TEST_F(TypeConverterTest, GetConverterTemplate) {
    // テンプレート版のget_converter
    auto int_conv = TypeConverter::get_converter<int>();
    EXPECT_EQ(int_conv("42").get<int>(), 42);
    
    auto float_conv = TypeConverter::get_converter<float>();
    EXPECT_DOUBLE_EQ(float_conv("3.14").get<double>(), 3.14);
    
    auto double_conv = TypeConverter::get_converter<double>();
    EXPECT_DOUBLE_EQ(double_conv("2.71").get<double>(), 2.71);
    
    auto bool_conv = TypeConverter::get_converter<bool>();
    EXPECT_TRUE(bool_conv("true").get<bool>());
    
    auto str_conv = TypeConverter::get_converter<std::string>();
    EXPECT_EQ(str_conv("test").get<std::string>(), "test");
}

// カスタム変換器のテスト
TEST_F(TypeConverterTest, CreateCustomConverter) {
    // カスタム変換関数: 文字列を大文字に変換
    auto upper_converter = [](const std::string& value) -> std::string {
        std::string result = value;
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    };
    
    auto custom_conv = TypeConverter::create_custom_converter<std::string>(upper_converter);
    
    EXPECT_EQ(custom_conv("hello").get<std::string>(), "HELLO");
    EXPECT_EQ(custom_conv("World").get<std::string>(), "WORLD");
}

TEST_F(TypeConverterTest, CustomConverterThrowsOnError) {
    // エラーを投げるカスタム変換器
    auto failing_converter = [](const std::string& value) -> int {
        if (value == "fail") {
            throw std::runtime_error("Custom error");
        }
        return std::stoi(value);
    };
    
    auto custom_conv = TypeConverter::create_custom_converter<int>(failing_converter);
    
    // 正常なケース
    EXPECT_EQ(custom_conv("42").get<int>(), 42);
    
    // エラーケース - カスタムエラーメッセージが含まれること
    try {
        custom_conv("fail");
        FAIL() << "Expected std::invalid_argument";
    } catch (const std::invalid_argument& e) {
        std::string error_msg = e.what();
        EXPECT_TRUE(error_msg.find("Custom error") != std::string::npos);
        EXPECT_TRUE(error_msg.find("fail") != std::string::npos);
    }
}

// エッジケースのテスト
TEST_F(TypeConverterTest, EdgeCases) {
    // 極端に長い文字列
    std::string long_string(1000, 'a');
    auto str_conv = TypeConverter::string_converter();
    EXPECT_EQ(str_conv(long_string).get<std::string>(), long_string);
    
    // Unicode文字（UTF-8）
    std::string unicode = "こんにちは世界";
    EXPECT_EQ(str_conv(unicode).get<std::string>(), unicode);
    
    // 特殊文字
    std::string special = "\t\n\r\"'\\";
    EXPECT_EQ(str_conv(special).get<std::string>(), special);
}