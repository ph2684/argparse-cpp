#include <gtest/gtest.h>
#include "../../include/argparse/argparse.hpp"

using namespace argparse::detail;

class TokenizerTest : public ::testing::Test {
protected:
    Tokenizer tokenizer;
    
    void SetUp() override {
        tokenizer = Tokenizer();
    }
};

// 基本的なトークン化のテスト
TEST_F(TokenizerTest, BasicTokenization) {
    std::vector<std::string> args = {"arg1", "--option", "value", "-v"};
    tokenizer.tokenize(args);
    
    EXPECT_EQ(tokenizer.size(), 4);
    
    Token token1 = tokenizer.next();
    EXPECT_EQ(token1.type, Token::POSITIONAL);
    EXPECT_EQ(token1.value, "arg1");
    
    Token token2 = tokenizer.next();
    EXPECT_EQ(token2.type, Token::LONG_OPTION);
    EXPECT_EQ(token2.value, "--option");
    
    Token token3 = tokenizer.next();
    EXPECT_EQ(token3.type, Token::POSITIONAL);
    EXPECT_EQ(token3.value, "value");
    
    Token token4 = tokenizer.next();
    EXPECT_EQ(token4.type, Token::SHORT_OPTION);
    EXPECT_EQ(token4.value, "-v");
}

// --key=valueの処理のテスト
TEST_F(TokenizerTest, LongOptionWithValue) {
    std::vector<std::string> args = {"--name=John", "--count=42"};
    tokenizer.tokenize(args);
    
    EXPECT_EQ(tokenizer.size(), 4);
    
    Token token1 = tokenizer.next();
    EXPECT_EQ(token1.type, Token::LONG_OPTION);
    EXPECT_EQ(token1.value, "--name");
    
    Token token2 = tokenizer.next();
    EXPECT_EQ(token2.type, Token::OPTION_VALUE);
    EXPECT_EQ(token2.value, "John");
    
    Token token3 = tokenizer.next();
    EXPECT_EQ(token3.type, Token::LONG_OPTION);
    EXPECT_EQ(token3.value, "--count");
    
    Token token4 = tokenizer.next();
    EXPECT_EQ(token4.type, Token::OPTION_VALUE);
    EXPECT_EQ(token4.value, "42");
}

// 引用符付き文字列のテスト
TEST_F(TokenizerTest, QuotedStrings) {
    std::vector<std::string> args = {"--message=\"Hello World\"", "--path='/tmp/test file'"};
    tokenizer.tokenize(args);
    
    EXPECT_EQ(tokenizer.size(), 4);
    
    Token token1 = tokenizer.next();
    EXPECT_EQ(token1.type, Token::LONG_OPTION);
    EXPECT_EQ(token1.value, "--message");
    
    Token token2 = tokenizer.next();
    EXPECT_EQ(token2.type, Token::OPTION_VALUE);
    EXPECT_EQ(token2.value, "Hello World");
    
    Token token3 = tokenizer.next();
    EXPECT_EQ(token3.type, Token::LONG_OPTION);
    EXPECT_EQ(token3.value, "--path");
    
    Token token4 = tokenizer.next();
    EXPECT_EQ(token4.type, Token::OPTION_VALUE);
    EXPECT_EQ(token4.value, "/tmp/test file");
}

// --による引数終了のテスト
TEST_F(TokenizerTest, EndOfOptions) {
    std::vector<std::string> args = {"--verbose", "--", "--not-an-option", "-x"};
    tokenizer.tokenize(args);
    
    EXPECT_EQ(tokenizer.size(), 4);
    
    Token token1 = tokenizer.next();
    EXPECT_EQ(token1.type, Token::LONG_OPTION);
    EXPECT_EQ(token1.value, "--verbose");
    
    Token token2 = tokenizer.next();
    EXPECT_EQ(token2.type, Token::END_OPTIONS);
    EXPECT_EQ(token2.value, "--");
    
    Token token3 = tokenizer.next();
    EXPECT_EQ(token3.type, Token::POSITIONAL);
    EXPECT_EQ(token3.value, "--not-an-option");
    
    Token token4 = tokenizer.next();
    EXPECT_EQ(token4.type, Token::POSITIONAL);
    EXPECT_EQ(token4.value, "-x");
}

// 複数の短縮形オプション（-abc）のテスト
TEST_F(TokenizerTest, CombinedShortOptions) {
    std::vector<std::string> args = {"-abc", "-v"};
    tokenizer.tokenize(args);
    
    EXPECT_EQ(tokenizer.size(), 4);
    
    Token token1 = tokenizer.next();
    EXPECT_EQ(token1.type, Token::SHORT_OPTION);
    EXPECT_EQ(token1.value, "-a");
    EXPECT_EQ(token1.raw_value, "-abc");
    
    Token token2 = tokenizer.next();
    EXPECT_EQ(token2.type, Token::SHORT_OPTION);
    EXPECT_EQ(token2.value, "-b");
    EXPECT_EQ(token2.raw_value, "-abc");
    
    Token token3 = tokenizer.next();
    EXPECT_EQ(token3.type, Token::SHORT_OPTION);
    EXPECT_EQ(token3.value, "-c");
    EXPECT_EQ(token3.raw_value, "-abc");
    
    Token token4 = tokenizer.next();
    EXPECT_EQ(token4.type, Token::SHORT_OPTION);
    EXPECT_EQ(token4.value, "-v");
}

// エスケープシーケンスのテスト
TEST_F(TokenizerTest, EscapeSequences) {
    std::vector<std::string> args = {"--message=\"Line 1\\nLine 2\\tTabbed\""};
    tokenizer.tokenize(args);
    
    EXPECT_EQ(tokenizer.size(), 2);
    
    Token token1 = tokenizer.next();
    EXPECT_EQ(token1.type, Token::LONG_OPTION);
    EXPECT_EQ(token1.value, "--message");
    
    Token token2 = tokenizer.next();
    EXPECT_EQ(token2.type, Token::OPTION_VALUE);
    EXPECT_EQ(token2.value, "Line 1\nLine 2\tTabbed");
}

// peek機能のテスト
TEST_F(TokenizerTest, PeekFunctionality) {
    std::vector<std::string> args = {"arg1", "--option"};
    tokenizer.tokenize(args);
    
    EXPECT_TRUE(tokenizer.has_next());
    
    Token peeked = tokenizer.peek();
    EXPECT_EQ(peeked.type, Token::POSITIONAL);
    EXPECT_EQ(peeked.value, "arg1");
    
    // peek後も位置は変わらない
    EXPECT_TRUE(tokenizer.has_next());
    EXPECT_EQ(tokenizer.position(), 0);
    
    Token consumed = tokenizer.next();
    EXPECT_EQ(consumed.type, Token::POSITIONAL);
    EXPECT_EQ(consumed.value, "arg1");
    EXPECT_EQ(tokenizer.position(), 1);
}

// reset機能のテスト
TEST_F(TokenizerTest, ResetFunctionality) {
    std::vector<std::string> args = {"arg1", "--option"};
    tokenizer.tokenize(args);
    
    tokenizer.next();  // 1つ消費
    EXPECT_EQ(tokenizer.position(), 1);
    
    tokenizer.reset();
    EXPECT_EQ(tokenizer.position(), 0);
    
    Token token = tokenizer.next();
    EXPECT_EQ(token.type, Token::POSITIONAL);
    EXPECT_EQ(token.value, "arg1");
}

// 空の引数リストのテスト
TEST_F(TokenizerTest, EmptyArgs) {
    std::vector<std::string> args = {};
    tokenizer.tokenize(args);
    
    EXPECT_EQ(tokenizer.size(), 0);
    EXPECT_FALSE(tokenizer.has_next());
}

// トークンが無いときの例外テスト
TEST_F(TokenizerTest, NoMoreTokensException) {
    std::vector<std::string> args = {"arg1"};
    tokenizer.tokenize(args);
    
    tokenizer.next();  // 唯一のトークンを消費
    
    EXPECT_FALSE(tokenizer.has_next());
    EXPECT_THROW(tokenizer.next(), std::runtime_error);
    EXPECT_THROW(tokenizer.peek(), std::runtime_error);
}

// seek機能のテスト
TEST_F(TokenizerTest, SeekFunctionality) {
    std::vector<std::string> args = {"arg1", "--option", "value"};
    tokenizer.tokenize(args);
    
    tokenizer.seek(2);
    EXPECT_EQ(tokenizer.position(), 2);
    
    Token token = tokenizer.next();
    EXPECT_EQ(token.type, Token::POSITIONAL);
    EXPECT_EQ(token.value, "value");
    
    // 範囲外にseekした場合のテスト
    tokenizer.seek(10);
    EXPECT_EQ(tokenizer.position(), 3);  // サイズで制限される
    EXPECT_FALSE(tokenizer.has_next());
}