#include <gtest/gtest.h>
#include "../../include/argparse/argparse.hpp"

class ArgumentParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        // テストの前処理
    }
    
    void TearDown() override {
        // テストの後処理
    }
};

// ArgumentParserの基本的な初期化テスト
TEST_F(ArgumentParserTest, BasicInitialization) {
    // デフォルトコンストラクタ
    argparse::ArgumentParser parser1;
    EXPECT_EQ(parser1.prog(), "program");
    EXPECT_EQ(parser1.description(), "");
    EXPECT_EQ(parser1.epilog(), "");
    EXPECT_TRUE(parser1.help_enabled());  // Default is now true
    
    // パラメータ付きコンストラクタ
    argparse::ArgumentParser parser2("myprogram", "This is a test program", "End of help");
    EXPECT_EQ(parser2.prog(), "myprogram");
    EXPECT_EQ(parser2.description(), "This is a test program");
    EXPECT_EQ(parser2.epilog(), "End of help");
    EXPECT_TRUE(parser2.help_enabled());  // Default is now true
    
    // ヘルプ無効化
    argparse::ArgumentParser parser3("", "", "", false);
    EXPECT_FALSE(parser3.help_enabled());
}

// add_argumentの基本動作テスト
TEST_F(ArgumentParserTest, AddArgument) {
    // add_help=falseで自動ヘルプを無効化
    argparse::ArgumentParser parser("test", "", "", false);
    
    // 位置引数の追加
    parser.add_argument("filename");
    EXPECT_EQ(parser.argument_count(), 1);
    EXPECT_TRUE(parser.has_argument("filename"));
    
    // オプション引数の追加（単一名前）
    parser.add_argument("--verbose");
    EXPECT_EQ(parser.argument_count(), 2);
    EXPECT_TRUE(parser.has_argument("--verbose"));
    
    // オプション引数の追加（短縮形と長形式）
    parser.add_argument("-o", "--output");
    EXPECT_EQ(parser.argument_count(), 3);
    EXPECT_TRUE(parser.has_argument("-o"));
    EXPECT_TRUE(parser.has_argument("--output"));
    
    // 複数名前での追加
    std::vector<std::string> names = {"-v", "--version", "--ver"};
    parser.add_argument(names);
    EXPECT_EQ(parser.argument_count(), 4);
    EXPECT_TRUE(parser.has_argument("-v"));
    EXPECT_TRUE(parser.has_argument("--version"));
    EXPECT_TRUE(parser.has_argument("--ver"));
}

// 引数の管理テスト
TEST_F(ArgumentParserTest, ArgumentManagement) {
    argparse::ArgumentParser parser("test");
    
    // 引数を追加してプロパティを設定
    auto& arg1 = parser.add_argument("filename").help("Input filename");
    auto& arg2 = parser.add_argument("--verbose").help("Enable verbose output");
    
    // 引数が正しく取得できることを確認
    auto retrieved_arg1 = parser.get_argument("filename");
    auto retrieved_arg2 = parser.get_argument("--verbose");
    
    ASSERT_NE(retrieved_arg1, nullptr);
    ASSERT_NE(retrieved_arg2, nullptr);
    
    EXPECT_EQ(retrieved_arg1->definition().help, "Input filename");
    EXPECT_EQ(retrieved_arg2->definition().help, "Enable verbose output");
    
    // 存在しない引数の取得
    auto non_existent = parser.get_argument("--nonexistent");
    EXPECT_EQ(non_existent, nullptr);
    EXPECT_FALSE(parser.has_argument("--nonexistent"));
}

// プログラム名の設定テスト
TEST_F(ArgumentParserTest, ProgramNameHandling) {
    argparse::ArgumentParser parser;
    
    // デフォルト名
    EXPECT_EQ(parser.prog(), "program");
    
    // 名前の設定
    parser.set_prog("myapp");
    EXPECT_EQ(parser.prog(), "myapp");
    
    // コンストラクタで指定した名前
    argparse::ArgumentParser parser2("specified_name");
    EXPECT_EQ(parser2.prog(), "specified_name");
}

// 引数の種類判定テスト
TEST_F(ArgumentParserTest, ArgumentTypeDetection) {
    argparse::ArgumentParser parser("test");
    
    // 位置引数
    auto& pos_arg = parser.add_argument("filename");
    EXPECT_TRUE(pos_arg.is_positional());
    
    // オプション引数（短縮形）
    auto& short_opt = parser.add_argument("-v");
    EXPECT_FALSE(short_opt.is_positional());
    
    // オプション引数（長形式）
    auto& long_opt = parser.add_argument("--verbose");
    EXPECT_FALSE(long_opt.is_positional());
    
    // 複数名前の場合（最初の名前で判定）
    auto& multi_opt = parser.add_argument("-o", "--output");
    EXPECT_FALSE(multi_opt.is_positional());
}

// 引数リストの取得テスト
TEST_F(ArgumentParserTest, ArgumentListAccess) {
    // add_help=falseで自動ヘルプを無効化
    argparse::ArgumentParser parser("test", "", "", false);
    
    EXPECT_EQ(parser.get_arguments().size(), 0);
    
    parser.add_argument("pos1");
    parser.add_argument("--opt1");
    parser.add_argument("-s", "--long");
    
    const auto& args = parser.get_arguments();
    EXPECT_EQ(args.size(), 3);
    
    // 追加順序で格納されていることを確認
    EXPECT_EQ(args[0]->get_name(), "pos1");
    EXPECT_EQ(args[1]->get_name(), "--opt1");
    EXPECT_EQ(args[2]->get_name(), "-s");
}