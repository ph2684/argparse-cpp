#include <gtest/gtest.h>
#include "../../include/argparse/argparse.hpp"

class NamespaceTest : public ::testing::Test {
protected:
    argparse::Namespace ns;
    
    void SetUp() override {
        // Setup test data
        ns.set("int_value", 42);
        ns.set("string_value", std::string("hello"));
        ns.set("double_value", 3.14);
        ns.set("bool_value", true);
    }
};

// 基本的な値の設定と取得のテスト
TEST_F(NamespaceTest, BasicSetAndGet) {
    // 整数値のテスト
    EXPECT_EQ(ns.get<int>("int_value"), 42);
    
    // 文字列値のテスト
    EXPECT_EQ(ns.get<std::string>("string_value"), "hello");
    
    // 浮動小数点値のテスト
    EXPECT_DOUBLE_EQ(ns.get<double>("double_value"), 3.14);
    
    // ブール値のテスト
    EXPECT_TRUE(ns.get<bool>("bool_value"));
}

// 型安全性の確認
TEST_F(NamespaceTest, TypeSafety) {
    // 正しい型でのアクセス
    EXPECT_NO_THROW(ns.get<int>("int_value"));
    
    // 間違った型でのアクセス（例外が投げられるべき）
    EXPECT_THROW(ns.get<std::string>("int_value"), std::runtime_error);
    EXPECT_THROW(ns.get<int>("string_value"), std::runtime_error);
    EXPECT_THROW(ns.get<double>("bool_value"), std::runtime_error);
}

// 存在しないキーでの例外テスト
TEST_F(NamespaceTest, NonExistentKey) {
    // 存在しないキーでのアクセス
    EXPECT_THROW(ns.get<int>("nonexistent"), std::runtime_error);
    EXPECT_THROW(ns.get<std::string>("missing_key"), std::runtime_error);
}

// has()メソッドの動作テスト
TEST_F(NamespaceTest, HasMethod) {
    // 存在するキー
    EXPECT_TRUE(ns.has("int_value"));
    EXPECT_TRUE(ns.has("string_value"));
    EXPECT_TRUE(ns.has("double_value"));
    EXPECT_TRUE(ns.has("bool_value"));
    
    // 存在しないキー
    EXPECT_FALSE(ns.has("nonexistent"));
    EXPECT_FALSE(ns.has("missing_key"));
}

// contains()メソッドの動作テスト（hasのエイリアス）
TEST_F(NamespaceTest, ContainsMethod) {
    // 存在するキー
    EXPECT_TRUE(ns.contains("int_value"));
    EXPECT_TRUE(ns.contains("string_value"));
    
    // 存在しないキー
    EXPECT_FALSE(ns.contains("nonexistent"));
}

// デフォルト値付きgetのテスト
TEST_F(NamespaceTest, GetWithDefault) {
    // 存在するキーの場合、デフォルト値は無視される
    EXPECT_EQ(ns.get<int>("int_value", 999), 42);
    
    // 存在しないキーの場合、デフォルト値が返される
    EXPECT_EQ(ns.get<int>("nonexistent", 123), 123);
    EXPECT_EQ(ns.get<std::string>("missing", "default"), "default");
}

// 空のNamespaceのテスト
TEST(NamespaceEmptyTest, EmptyNamespace) {
    argparse::Namespace empty_ns;
    
    EXPECT_TRUE(empty_ns.empty());
    EXPECT_EQ(empty_ns.size(), 0);
    EXPECT_FALSE(empty_ns.has("any_key"));
    
    // デフォルト値付きアクセス
    EXPECT_EQ(empty_ns.get<int>("any_key", 100), 100);
}

// keys()メソッドのテスト
TEST_F(NamespaceTest, KeysMethod) {
    std::vector<std::string> keys = ns.keys();
    
    EXPECT_EQ(keys.size(), 4);
    
    // すべてのキーが含まれているかチェック
    std::sort(keys.begin(), keys.end());
    std::vector<std::string> expected = {"bool_value", "double_value", "int_value", "string_value"};
    EXPECT_EQ(keys, expected);
}

// size()とempty()のテスト
TEST_F(NamespaceTest, SizeAndEmpty) {
    EXPECT_EQ(ns.size(), 4);
    EXPECT_FALSE(ns.empty());
    
    // クリア後のテスト
    ns.clear();
    EXPECT_EQ(ns.size(), 0);
    EXPECT_TRUE(ns.empty());
}

// remove()メソッドのテスト
TEST_F(NamespaceTest, RemoveMethod) {
    EXPECT_TRUE(ns.has("int_value"));
    
    // 存在するキーの削除
    EXPECT_TRUE(ns.remove("int_value"));
    EXPECT_FALSE(ns.has("int_value"));
    EXPECT_EQ(ns.size(), 3);
    
    // 存在しないキーの削除
    EXPECT_FALSE(ns.remove("nonexistent"));
    EXPECT_EQ(ns.size(), 3);
}

// コピーコンストラクタとムーブコンストラクタのテスト
TEST_F(NamespaceTest, CopyAndMoveConstructors) {
    // コピーコンストラクタ
    argparse::Namespace copied_ns(ns);
    EXPECT_EQ(copied_ns.get<int>("int_value"), 42);
    EXPECT_EQ(copied_ns.size(), 4);
    
    // ムーブコンストラクタ
    argparse::Namespace moved_ns(std::move(copied_ns));
    EXPECT_EQ(moved_ns.get<int>("int_value"), 42);
    EXPECT_EQ(moved_ns.size(), 4);
}

// 代入演算子のテスト
TEST_F(NamespaceTest, AssignmentOperators) {
    argparse::Namespace new_ns;
    
    // コピー代入
    new_ns = ns;
    EXPECT_EQ(new_ns.get<int>("int_value"), 42);
    EXPECT_EQ(new_ns.size(), 4);
    
    // ムーブ代入
    argparse::Namespace another_ns;
    another_ns = std::move(new_ns);
    EXPECT_EQ(another_ns.get<int>("int_value"), 42);
    EXPECT_EQ(another_ns.size(), 4);
}

// 上書き設定のテスト
TEST_F(NamespaceTest, OverwriteValues) {
    EXPECT_EQ(ns.get<int>("int_value"), 42);
    
    // 同じキーに異なる値を設定
    ns.set("int_value", 100);
    EXPECT_EQ(ns.get<int>("int_value"), 100);
    
    // 異なる型での上書き（型安全性は保たれるべき）
    ns.set("int_value", std::string("now_string"));
    EXPECT_EQ(ns.get<std::string>("int_value"), "now_string");
    EXPECT_THROW(ns.get<int>("int_value"), std::runtime_error);
}

// raw値アクセスのテスト
TEST_F(NamespaceTest, RawValueAccess) {
    // raw値の取得
    const argparse::detail::AnyValue& raw_value = ns.get_raw("int_value");
    EXPECT_EQ(raw_value.get<int>(), 42);
    
    // raw値の設定
    argparse::detail::AnyValue new_value(999);
    ns.set_raw("new_raw_value", new_value);
    EXPECT_EQ(ns.get<int>("new_raw_value"), 999);
    
    // ムーブ版の設定
    argparse::detail::AnyValue move_value(777);
    ns.set_raw("move_raw_value", std::move(move_value));
    EXPECT_EQ(ns.get<int>("move_raw_value"), 777);
}