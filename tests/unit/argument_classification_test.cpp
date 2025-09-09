#include <gtest/gtest.h>
#include "../../include/argparse/argparse.hpp"

class ArgumentClassificationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // テストの前処理
    }
    
    void TearDown() override {
        // テストの後処理
    }
};

// 位置引数の判定テスト
TEST_F(ArgumentClassificationTest, PositionalArgumentDetection) {
    argparse::ArgumentParser parser("test");
    
    // 位置引数（普通の名前）
    auto& arg1 = parser.add_argument("filename");
    EXPECT_TRUE(arg1.is_positional());
    
    // 位置引数（アンダースコアで始まる）
    auto& arg2 = parser.add_argument("_temp");
    EXPECT_TRUE(arg2.is_positional());
    
    // 位置引数（数字を含む、但し先頭は英字）
    auto& arg3 = parser.add_argument("file123");
    EXPECT_TRUE(arg3.is_positional());
}

// オプション引数の判定テスト
TEST_F(ArgumentClassificationTest, OptionalArgumentDetection) {
    argparse::ArgumentParser parser("test");
    
    // 短縮形オプション
    auto& arg1 = parser.add_argument("-v");
    EXPECT_FALSE(arg1.is_positional());
    
    // 短縮形オプション（複数文字）
    auto& arg2 = parser.add_argument("-ab");
    EXPECT_FALSE(arg2.is_positional());
    
    // 長形式オプション
    auto& arg3 = parser.add_argument("--verbose");
    EXPECT_FALSE(arg3.is_positional());
    
    // 長形式オプション（短い）
    auto& arg4 = parser.add_argument("--v");
    EXPECT_FALSE(arg4.is_positional());
}

// 短縮形と長形式の処理テスト
TEST_F(ArgumentClassificationTest, ShortAndLongFormHandling) {
    argparse::ArgumentParser parser("test");
    
    // 短縮形と長形式の組み合わせ
    auto& arg = parser.add_argument("-v", "--verbose");
    EXPECT_FALSE(arg.is_positional());
    
    // 両方の名前が登録されていることを確認
    EXPECT_TRUE(parser.has_argument("-v"));
    EXPECT_TRUE(parser.has_argument("--verbose"));
    
    // 同じArgumentオブジェクトが返されることを確認
    auto ptr1 = parser.get_argument("-v");
    auto ptr2 = parser.get_argument("--verbose");
    EXPECT_EQ(ptr1, ptr2);
    
    // 名前リストが正しいことを確認
    const auto& names = arg.get_names();
    EXPECT_EQ(names.size(), 2);
    EXPECT_EQ(names[0], "-v");
    EXPECT_EQ(names[1], "--verbose");
}

// 複数名前での処理テスト
TEST_F(ArgumentClassificationTest, MultipleNamesHandling) {
    // add_help=falseで自動ヘルプを無効化
    argparse::ArgumentParser parser("test", "", "", false);
    
    // 複数のエイリアス（-hの代わりに-xを使用）
    std::vector<std::string> names = {"-x", "--example", "--ex"};
    auto& arg = parser.add_argument(names);
    
    EXPECT_FALSE(arg.is_positional());
    
    // すべての名前が登録されていることを確認
    for (const auto& name : names) {
        EXPECT_TRUE(parser.has_argument(name));
        auto ptr = parser.get_argument(name);
        EXPECT_NE(ptr, nullptr);
    }
    
    // すべて同じオブジェクトを参照していることを確認
    auto ptr1 = parser.get_argument("-x");
    auto ptr2 = parser.get_argument("--example");
    auto ptr3 = parser.get_argument("--ex");
    EXPECT_EQ(ptr1, ptr2);
    EXPECT_EQ(ptr2, ptr3);
}

// 重複チェックテスト
TEST_F(ArgumentClassificationTest, DuplicateArgumentCheck) {
    argparse::ArgumentParser parser("test");
    
    // 最初の引数を追加
    parser.add_argument("--verbose");
    EXPECT_TRUE(parser.has_argument("--verbose"));
    
    // 同じ名前の引数を再度追加しようとした場合は例外が投げられる
    EXPECT_THROW(parser.add_argument("--verbose"), std::runtime_error);
    
    // 短縮形と長形式の重複チェック
    parser.add_argument("-v", "--version");
    EXPECT_THROW(parser.add_argument("--version"), std::runtime_error);
    EXPECT_THROW(parser.add_argument("-v"), std::runtime_error);
}

// エッジケースのテスト
TEST_F(ArgumentClassificationTest, EdgeCases) {
    argparse::ArgumentParser parser("test");
    
    // 空の文字列（不正な引数名） - 例外が投げられる
    EXPECT_THROW(parser.add_argument(""), std::invalid_argument);
    
    // ハイフンのみ（不正な引数名） - 例外が投げられる
    EXPECT_THROW(parser.add_argument("-"), std::invalid_argument);
    
    // ダブルハイフンのみ（不正な引数名） - 例外が投げられる
    EXPECT_THROW(parser.add_argument("--"), std::invalid_argument);
}

// POSIX準拠の引数形式テスト
TEST_F(ArgumentClassificationTest, POSIXCompliantArgumentFormat) {
    argparse::ArgumentParser parser("test");
    
    // 正しい短縮形
    auto& arg1 = parser.add_argument("-f");
    EXPECT_FALSE(arg1.is_positional());
    
    // 正しい長形式
    auto& arg2 = parser.add_argument("--file");
    EXPECT_FALSE(arg2.is_positional());
    
    // 数字を含む引数名
    auto& arg3 = parser.add_argument("--level-2");
    EXPECT_FALSE(arg3.is_positional());
    
    // アンダースコアを含む引数名
    auto& arg4 = parser.add_argument("--output_file");
    EXPECT_FALSE(arg4.is_positional());
}

// 引数の順序と型の混合テスト
TEST_F(ArgumentClassificationTest, MixedArgumentTypes) {
    // add_help=falseで自動ヘルプを無効化
    argparse::ArgumentParser parser("test", "", "", false);
    
    // 位置引数、短縮形、長形式、組み合わせを混在
    auto& pos1 = parser.add_argument("input");
    auto& opt1 = parser.add_argument("-v");
    auto& opt2 = parser.add_argument("--output");
    auto& opt3 = parser.add_argument("-f", "--force");
    auto& pos2 = parser.add_argument("destination");
    
    // 型の判定が正しいことを確認
    EXPECT_TRUE(pos1.is_positional());
    EXPECT_FALSE(opt1.is_positional());
    EXPECT_FALSE(opt2.is_positional());
    EXPECT_FALSE(opt3.is_positional());
    EXPECT_TRUE(pos2.is_positional());
    
    // カウントが正しいことを確認
    EXPECT_EQ(parser.argument_count(), 5);
    
    // すべての名前が登録されていることを確認
    EXPECT_TRUE(parser.has_argument("input"));
    EXPECT_TRUE(parser.has_argument("-v"));
    EXPECT_TRUE(parser.has_argument("--output"));
    EXPECT_TRUE(parser.has_argument("-f"));
    EXPECT_TRUE(parser.has_argument("--force"));
    EXPECT_TRUE(parser.has_argument("destination"));
}

// 無効な引数名のテスト
TEST_F(ArgumentClassificationTest, InvalidArgumentNames) {
    argparse::ArgumentParser parser("test");
    
    // 位置引数で無効な名前
    EXPECT_THROW(parser.add_argument("123invalid"), std::invalid_argument);  // 数字で始まる
    EXPECT_THROW(parser.add_argument("invalid@name"), std::invalid_argument); // 無効な文字
    EXPECT_THROW(parser.add_argument("invalid space"), std::invalid_argument); // スペース
    
    // 短縮形オプションで無効な名前
    EXPECT_THROW(parser.add_argument("-@"), std::invalid_argument);          // 無効な文字
    EXPECT_THROW(parser.add_argument("- "), std::invalid_argument);          // スペース
    
    // 長形式オプションで無効な名前
    EXPECT_THROW(parser.add_argument("--invalid@option"), std::invalid_argument); // 無効な文字
    EXPECT_THROW(parser.add_argument("--invalid option"), std::invalid_argument); // スペース
    
    // 正当な引数名は通る
    EXPECT_NO_THROW(parser.add_argument("valid_name"));
    EXPECT_NO_THROW(parser.add_argument("-v"));
    EXPECT_NO_THROW(parser.add_argument("--valid-option"));
    EXPECT_NO_THROW(parser.add_argument("--valid_option"));
    EXPECT_NO_THROW(parser.add_argument("_private"));
}