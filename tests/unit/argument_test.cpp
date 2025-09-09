#include <gtest/gtest.h>
#include "../../include/argparse/argparse.hpp"

using namespace argparse;

class ArgumentTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    
    void TearDown() override {
    }
};

// 基本的なコンストラクタのテスト
TEST_F(ArgumentTest, DefaultConstructor) {
    Argument arg;
    
    EXPECT_TRUE(arg.get_names().empty());
    EXPECT_EQ(arg.get_name(), "");
    EXPECT_FALSE(arg.is_positional());  // 名前がない場合は位置引数扱いではない
}

TEST_F(ArgumentTest, ConstructorWithSingleName) {
    Argument arg("--verbose");
    
    EXPECT_EQ(arg.get_names().size(), 1);
    EXPECT_EQ(arg.get_name(), "--verbose");
    EXPECT_FALSE(arg.is_positional());
}

TEST_F(ArgumentTest, ConstructorWithMultipleNames) {
    std::vector<std::string> names = {"--verbose", "-v"};
    Argument arg(names);
    
    EXPECT_EQ(arg.get_names().size(), 2);
    EXPECT_EQ(arg.get_name(), "--verbose");
    EXPECT_FALSE(arg.is_positional());
}

// 位置引数の判定テスト
TEST_F(ArgumentTest, IsPositional) {
    Argument pos_arg("filename");
    EXPECT_TRUE(pos_arg.is_positional());
    
    Argument opt_arg("--output");
    EXPECT_FALSE(opt_arg.is_positional());
    
    Argument short_arg("-o");
    EXPECT_FALSE(short_arg.is_positional());
    
    // 空の名前は位置引数扱いではない
    Argument empty_arg("");
    EXPECT_FALSE(empty_arg.is_positional());
}

// Fluent interface のテスト
TEST_F(ArgumentTest, FluentInterface) {
    Argument arg("--count");
    
    // チェーン可能であることを確認
    Argument& result = arg.help("Number of iterations")
                         .type("int")
                         .default_value(1)
                         .required(true);
    
    // 戻り値が同じオブジェクトであることを確認
    EXPECT_EQ(&result, &arg);
    
    // 設定された値を確認
    const auto& def = arg.definition();
    EXPECT_EQ(def.help, "Number of iterations");
    EXPECT_EQ(def.type_name, "int");
    EXPECT_TRUE(def.required);
}

// Help設定のテスト
TEST_F(ArgumentTest, Help) {
    Argument arg("--verbose");
    arg.help("Enable verbose output");
    
    EXPECT_EQ(arg.definition().help, "Enable verbose output");
}

// Metavar設定のテスト
TEST_F(ArgumentTest, Metavar) {
    Argument arg("--output");
    arg.metavar("FILE");
    
    EXPECT_EQ(arg.definition().metavar, "FILE");
}

// Action設定のテスト
TEST_F(ArgumentTest, Action) {
    Argument arg("--verbose");
    arg.action("store_true");
    
    EXPECT_EQ(arg.definition().action, "store_true");
}

// Type設定のテスト
TEST_F(ArgumentTest, Type) {
    Argument arg("--count");
    arg.type("int");
    
    EXPECT_EQ(arg.definition().type_name, "int");
    
    // 型変換器が設定されていることを確認
    EXPECT_TRUE(arg.definition().converter != nullptr);
}

// Default value設定のテスト
TEST_F(ArgumentTest, DefaultValue) {
    Argument arg("--count");
    arg.default_value(42);
    
    EXPECT_FALSE(arg.definition().default_value.empty());
    EXPECT_EQ(arg.definition().default_value.get<int>(), 42);
}

TEST_F(ArgumentTest, DefaultValueString) {
    Argument arg("--name");
    arg.default_value(std::string("default"));
    
    EXPECT_FALSE(arg.definition().default_value.empty());
    EXPECT_EQ(arg.definition().default_value.get<std::string>(), "default");
}

// Choices設定のテスト
TEST_F(ArgumentTest, Choices) {
    Argument arg("--format");
    std::vector<std::string> formats = {"json", "xml", "csv"};
    arg.choices(formats);
    
    const auto& choices = arg.definition().choices;
    EXPECT_EQ(choices.size(), 3);
    EXPECT_EQ(choices[0].get<std::string>(), "json");
    EXPECT_EQ(choices[1].get<std::string>(), "xml");
    EXPECT_EQ(choices[2].get<std::string>(), "csv");
}

TEST_F(ArgumentTest, ChoicesInt) {
    Argument arg("--level");
    std::vector<int> levels = {1, 2, 3, 4, 5};
    arg.choices(levels);
    
    const auto& choices = arg.definition().choices;
    EXPECT_EQ(choices.size(), 5);
    EXPECT_EQ(choices[0].get<int>(), 1);
    EXPECT_EQ(choices[4].get<int>(), 5);
}

// Nargs設定のテスト
TEST_F(ArgumentTest, NargsInt) {
    Argument arg("filenames");
    arg.nargs(3);
    
    EXPECT_EQ(arg.definition().nargs, 3);
}

TEST_F(ArgumentTest, NargsString) {
    Argument arg("files");
    
    arg.nargs("?");
    EXPECT_EQ(arg.definition().nargs, -2);  // Optional
    
    arg.nargs("*");
    EXPECT_EQ(arg.definition().nargs, -3);  // Zero or more
    
    arg.nargs("+");
    EXPECT_EQ(arg.definition().nargs, -4);  // One or more
}

// Required設定のテスト
TEST_F(ArgumentTest, Required) {
    Argument arg("--output");
    
    // デフォルトはfalse
    EXPECT_FALSE(arg.definition().required);
    
    arg.required(true);
    EXPECT_TRUE(arg.definition().required);
    
    arg.required(false);
    EXPECT_FALSE(arg.definition().required);
    
    // 引数なしでrequired()を呼ぶとtrueになる
    arg.required();
    EXPECT_TRUE(arg.definition().required);
}

// Value conversion のテスト
TEST_F(ArgumentTest, ConvertValue) {
    Argument int_arg("--count");
    int_arg.type("int");
    
    auto result = int_arg.convert_value("42");
    EXPECT_EQ(result.get<int>(), 42);
    
    // デフォルトconverterはstring
    Argument str_arg("--name");
    auto str_result = str_arg.convert_value("test");
    EXPECT_EQ(str_result.get<std::string>(), "test");
}

TEST_F(ArgumentTest, ConvertValueThrowsOnError) {
    Argument int_arg("--count");
    int_arg.type("int");
    
    EXPECT_THROW(int_arg.convert_value("invalid"), std::invalid_argument);
}

// カスタム変換器のテスト
TEST_F(ArgumentTest, CustomConverter) {
    Argument arg("--upper");
    
    // 文字列を大文字に変換するカスタム変換器
    arg.converter<std::string>([](const std::string& value) -> std::string {
        std::string result = value;
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    });
    
    auto result = arg.convert_value("hello");
    EXPECT_EQ(result.get<std::string>(), "HELLO");
}

TEST_F(ArgumentTest, DirectCustomConverter) {
    Argument arg("--test");
    
    // 直接的なconverter設定
    arg.converter([](const std::string& value) -> detail::AnyValue {
        return detail::AnyValue(value + "_converted");
    });
    
    auto result = arg.convert_value("test");
    EXPECT_EQ(result.get<std::string>(), "test_converted");
}

// Validation のテスト
TEST_F(ArgumentTest, ValidateValueWithChoices) {
    Argument arg("--format");
    std::vector<std::string> formats = {"json", "xml", "csv"};
    arg.choices(formats);
    
    detail::AnyValue valid_value(std::string("json"));
    detail::AnyValue invalid_value(std::string("yaml"));
    
    // choicesが実装されているので、実際の値比較が行われる
    EXPECT_TRUE(arg.validate_value(valid_value));   // 有効な値
    EXPECT_FALSE(arg.validate_value(invalid_value));  // 無効な値
}

TEST_F(ArgumentTest, ValidateValueWithoutChoices) {
    Argument arg("--name");
    
    detail::AnyValue value(std::string("any_value"));
    EXPECT_TRUE(arg.validate_value(value));
}

// 複合的なテスト
TEST_F(ArgumentTest, ComplexConfiguration) {
    Argument arg("--level");
    
    arg.help("Set logging level")
       .type("int")
       .default_value(1)
       .choices(std::vector<int>{1, 2, 3, 4, 5})
       .required(false)
       .metavar("LEVEL");
    
    const auto& def = arg.definition();
    EXPECT_EQ(def.help, "Set logging level");
    EXPECT_EQ(def.type_name, "int");
    EXPECT_EQ(def.default_value.get<int>(), 1);
    EXPECT_EQ(def.choices.size(), 5);
    EXPECT_FALSE(def.required);
    EXPECT_EQ(def.metavar, "LEVEL");
    
    // 値の変換が正しく動作することを確認
    auto converted = arg.convert_value("3");
    EXPECT_EQ(converted.get<int>(), 3);
}

// ArgumentDefinition のデフォルト値テスト
TEST_F(ArgumentTest, ArgumentDefinitionDefaults) {
    ArgumentDefinition def;
    
    EXPECT_TRUE(def.names.empty());
    EXPECT_TRUE(def.help.empty());
    EXPECT_TRUE(def.metavar.empty());
    EXPECT_EQ(def.action, "store");
    EXPECT_EQ(def.type_name, "string");
    EXPECT_TRUE(def.default_value.empty());
    EXPECT_TRUE(def.choices.empty());
    EXPECT_EQ(def.nargs, 1);
    EXPECT_FALSE(def.required);
    EXPECT_EQ(def.converter, nullptr);
    EXPECT_EQ(def.validator, nullptr);
}

// Immutable access のテスト
TEST_F(ArgumentTest, ImmutableAccess) {
    Argument arg("--test");
    arg.help("Test help");
    
    const Argument& const_arg = arg;
    const auto& def = const_arg.definition();
    
    EXPECT_EQ(def.help, "Test help");
}

// エッジケースのテスト
TEST_F(ArgumentTest, EdgeCases) {
    // 空の名前リスト
    std::vector<std::string> empty_names;
    Argument arg1(empty_names);
    EXPECT_TRUE(arg1.get_names().empty());
    EXPECT_EQ(arg1.get_name(), "");
    
    // 複数の空文字列
    std::vector<std::string> empty_strings = {"", ""};
    Argument arg2(empty_strings);
    EXPECT_EQ(arg2.get_names().size(), 2);
    EXPECT_EQ(arg2.get_name(), "");
    EXPECT_FALSE(arg2.is_positional());  // 空文字列は位置引数扱いではない
    
    // 非常に長い名前
    std::string long_name(1000, 'a');
    Argument arg3(long_name);
    EXPECT_EQ(arg3.get_name(), long_name);
}