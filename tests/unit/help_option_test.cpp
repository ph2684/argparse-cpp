#include <gtest/gtest.h>
#include "../../include/argparse/argparse.hpp"
#include <sstream>
#include <iostream>

class HelpOptionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // テストの前処理
    }
    
    void TearDown() override {
        // テストの後処理
    }
};

// add_helpのデフォルト動作テスト
TEST_F(HelpOptionTest, DefaultAddHelpBehavior) {
    // デフォルトでadd_help=trueになることを確認
    argparse::ArgumentParser parser1;
    EXPECT_TRUE(parser1.help_enabled());
    
    // 明示的にadd_help=trueを指定
    argparse::ArgumentParser parser2("test", "description", "epilog", true);
    EXPECT_TRUE(parser2.help_enabled());
    
    // add_help=falseを指定
    argparse::ArgumentParser parser3("test", "description", "epilog", false);
    EXPECT_FALSE(parser3.help_enabled());
}

// --help/-hオプションの自動追加テスト
TEST_F(HelpOptionTest, HelpOptionAutoAddition) {
    argparse::ArgumentParser parser("test", "Test program", "End of help", true);
    
    // --helpと-hが自動的に追加されていることを確認
    EXPECT_TRUE(parser.has_argument("--help"));
    EXPECT_TRUE(parser.has_argument("-h"));
    
    // ヘルプ引数が取得できることを確認
    auto help_arg = parser.get_argument("--help");
    ASSERT_NE(help_arg, nullptr);
    EXPECT_EQ(help_arg->definition().action, "help");
    EXPECT_EQ(help_arg->definition().help, "show this help message and exit");
}

// add_help=falseの場合はヘルプオプションが追加されないことをテスト
TEST_F(HelpOptionTest, NoHelpOptionWhenDisabled) {
    argparse::ArgumentParser parser("test", "Test program", "End of help", false);
    
    // --helpと-hが追加されていないことを確認
    EXPECT_FALSE(parser.has_argument("--help"));
    EXPECT_FALSE(parser.has_argument("-h"));
}

// ユーザー定義のヘルプオプションの優先度テスト
TEST_F(HelpOptionTest, UserDefinedHelpPriority) {
    // add_help=falseでパーサーを作成してからユーザーが--helpを定義
    argparse::ArgumentParser parser("test", "Test program", "End of help", false);
    
    // ユーザーが--helpを定義
    parser.add_argument("--help").help("Custom help message");
    
    // ユーザー定義のヘルプが設定されていることを確認
    auto help_arg = parser.get_argument("--help");
    ASSERT_NE(help_arg, nullptr);
    EXPECT_EQ(help_arg->definition().help, "Custom help message");
    
    // デフォルトのヘルプ処理は追加されていないことを確認
    EXPECT_FALSE(parser.help_enabled());
}

// ヘルプオプションの長形式と短形式の両方が機能することをテスト
TEST_F(HelpOptionTest, BothHelpOptionsWork) {
    argparse::ArgumentParser parser("test", "Test program", "End of help", true);
    
    // --helpと-hの両方が同じ引数を指していることを確認
    auto help_long = parser.get_argument("--help");
    auto help_short = parser.get_argument("-h");
    
    ASSERT_NE(help_long, nullptr);
    ASSERT_NE(help_short, nullptr);
    EXPECT_EQ(help_long, help_short);  // 同じArgumentオブジェクトを指している
}

// ヘルプオプション実行時の動作テスト（exit(0)呼び出しを確認）
// 注意: exit(0)が呼ばれるためプロセスが終了するので、
// 実際のテストでは専用のフレームワークまたはモック化が必要
TEST_F(HelpOptionTest, HelpDisplayTest) {
    argparse::ArgumentParser parser("testprog", "This is a test program", "End of help text", true);
    parser.add_argument("filename").help("Input file name");
    parser.add_argument("-v", "--verbose").action("store_true").help("Enable verbose output");
    
    // exit(0)が呼ばれることをテストするのは困難なので、
    // ここではhelpGeneratorが正しく動作することを確認
    // 実際のヘルプ表示テストは統合テストで行う方が適切
    
    // ヘルプが有効であることを確認
    EXPECT_TRUE(parser.help_enabled());
    
    // ヘルプ引数が正しく設定されていることを確認
    auto help_arg = parser.get_argument("--help");
    ASSERT_NE(help_arg, nullptr);
    EXPECT_EQ(help_arg->definition().action, "help");
}

// ヘルプ生成内容の基本テスト
TEST_F(HelpOptionTest, HelpContentGeneration) {
    argparse::ArgumentParser parser("testprog", "This is a test program", "End of help text", true);
    parser.add_argument("filename").help("Input file name");
    parser.add_argument("-v", "--verbose").action("store_true").help("Enable verbose output");
    
    // HelpGeneratorを直接呼び出してヘルプメッセージを生成
    std::string help_message = argparse::detail::HelpGenerator::generate_help(parser);
    
    // ヘルプメッセージに期待する要素が含まれていることを確認
    EXPECT_NE(help_message.find("usage: testprog"), std::string::npos);
    EXPECT_NE(help_message.find("This is a test program"), std::string::npos);
    EXPECT_NE(help_message.find("End of help text"), std::string::npos);
    EXPECT_NE(help_message.find("filename"), std::string::npos);
    EXPECT_NE(help_message.find("-v, --verbose"), std::string::npos);
    EXPECT_NE(help_message.find("-h, --help"), std::string::npos);
    EXPECT_NE(help_message.find("show this help message and exit"), std::string::npos);
}