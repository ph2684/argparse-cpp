#include <gtest/gtest.h>
#include "../../include/argparse/argparse.hpp"

using namespace argparse::detail;

class AnyValueTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    
    void TearDown() override {
    }
};

TEST_F(AnyValueTest, DefaultConstructor) {
    AnyValue value;
    EXPECT_TRUE(value.empty());
    EXPECT_EQ(value.type(), typeid(void));
}

TEST_F(AnyValueTest, ConstructFromInt) {
    AnyValue value(42);
    EXPECT_FALSE(value.empty());
    EXPECT_EQ(value.type(), typeid(int));
    EXPECT_EQ(value.get<int>(), 42);
}

TEST_F(AnyValueTest, ConstructFromString) {
    std::string test_str = "hello";
    AnyValue value(test_str);
    EXPECT_FALSE(value.empty());
    EXPECT_EQ(value.type(), typeid(std::string));
    EXPECT_EQ(value.get<std::string>(), "hello");
}

TEST_F(AnyValueTest, ConstructFromCString) {
    const char* cstr = "hello world";
    AnyValue value(cstr);
    EXPECT_FALSE(value.empty());
    EXPECT_EQ(value.type(), typeid(const char*));
    EXPECT_STREQ(value.get<const char*>(), "hello world");
}

TEST_F(AnyValueTest, ConstructFromDouble) {
    AnyValue value(3.14159);
    EXPECT_FALSE(value.empty());
    EXPECT_EQ(value.type(), typeid(double));
    EXPECT_DOUBLE_EQ(value.get<double>(), 3.14159);
}

TEST_F(AnyValueTest, ConstructFromBool) {
    AnyValue value_true(true);
    AnyValue value_false(false);
    
    EXPECT_FALSE(value_true.empty());
    EXPECT_EQ(value_true.type(), typeid(bool));
    EXPECT_TRUE(value_true.get<bool>());
    
    EXPECT_FALSE(value_false.empty());
    EXPECT_EQ(value_false.type(), typeid(bool));
    EXPECT_FALSE(value_false.get<bool>());
}

TEST_F(AnyValueTest, CopyConstructor) {
    AnyValue original(42);
    AnyValue copy(original);
    
    EXPECT_FALSE(copy.empty());
    EXPECT_EQ(copy.type(), typeid(int));
    EXPECT_EQ(copy.get<int>(), 42);
    
    // 元の値を変更しても影響しないことを確認
    original = std::string("changed");
    EXPECT_EQ(copy.get<int>(), 42);
}

TEST_F(AnyValueTest, MoveConstructor) {
    AnyValue original(42);
    AnyValue moved(std::move(original));
    
    EXPECT_FALSE(moved.empty());
    EXPECT_EQ(moved.type(), typeid(int));
    EXPECT_EQ(moved.get<int>(), 42);
    
    // 元の値は空になっている
    EXPECT_TRUE(original.empty());
}

TEST_F(AnyValueTest, CopyAssignment) {
    AnyValue value1(42);
    AnyValue value2;
    
    value2 = value1;
    
    EXPECT_FALSE(value2.empty());
    EXPECT_EQ(value2.type(), typeid(int));
    EXPECT_EQ(value2.get<int>(), 42);
}

TEST_F(AnyValueTest, MoveAssignment) {
    AnyValue value1(42);
    AnyValue value2;
    
    value2 = std::move(value1);
    
    EXPECT_FALSE(value2.empty());
    EXPECT_EQ(value2.type(), typeid(int));
    EXPECT_EQ(value2.get<int>(), 42);
    
    EXPECT_TRUE(value1.empty());
}

TEST_F(AnyValueTest, ValueAssignment) {
    AnyValue value;
    
    value = 42;
    EXPECT_FALSE(value.empty());
    EXPECT_EQ(value.type(), typeid(int));
    EXPECT_EQ(value.get<int>(), 42);
    
    value = std::string("test");
    EXPECT_FALSE(value.empty());
    EXPECT_EQ(value.type(), typeid(std::string));
    EXPECT_EQ(value.get<std::string>(), "test");
}

TEST_F(AnyValueTest, SelfAssignment) {
    AnyValue value(42);
    value = value;  // 自己代入
    
    EXPECT_FALSE(value.empty());
    EXPECT_EQ(value.type(), typeid(int));
    EXPECT_EQ(value.get<int>(), 42);
}

TEST_F(AnyValueTest, GetThrowsOnEmpty) {
    AnyValue value;
    
    EXPECT_THROW(value.get<int>(), std::runtime_error);
}

TEST_F(AnyValueTest, GetThrowsOnTypeMismatch) {
    AnyValue value(42);
    
    EXPECT_THROW(value.get<std::string>(), std::runtime_error);
    EXPECT_THROW(value.get<double>(), std::runtime_error);
}

TEST_F(AnyValueTest, ConstGet) {
    const AnyValue value(42);
    
    EXPECT_FALSE(value.empty());
    EXPECT_EQ(value.type(), typeid(int));
    EXPECT_EQ(value.get<int>(), 42);
}

TEST_F(AnyValueTest, ConstGetThrowsOnEmpty) {
    const AnyValue value;
    
    EXPECT_THROW(value.get<int>(), std::runtime_error);
}

TEST_F(AnyValueTest, ConstGetThrowsOnTypeMismatch) {
    const AnyValue value(42);
    
    EXPECT_THROW(value.get<std::string>(), std::runtime_error);
}

TEST_F(AnyValueTest, Reset) {
    AnyValue value(42);
    EXPECT_FALSE(value.empty());
    
    value.reset();
    EXPECT_TRUE(value.empty());
    EXPECT_EQ(value.type(), typeid(void));
}

TEST_F(AnyValueTest, Swap) {
    AnyValue value1(42);
    AnyValue value2(std::string("test"));
    
    value1.swap(value2);
    
    EXPECT_EQ(value1.type(), typeid(std::string));
    EXPECT_EQ(value1.get<std::string>(), "test");
    
    EXPECT_EQ(value2.type(), typeid(int));
    EXPECT_EQ(value2.get<int>(), 42);
}

TEST_F(AnyValueTest, SwapWithEmpty) {
    AnyValue value1(42);
    AnyValue value2;
    
    value1.swap(value2);
    
    EXPECT_TRUE(value1.empty());
    EXPECT_FALSE(value2.empty());
    EXPECT_EQ(value2.get<int>(), 42);
}

// 実際の型消去が動作することを確認
TEST_F(AnyValueTest, TypeErasureWorksCorrectly) {
    std::vector<AnyValue> values;
    
    values.push_back(AnyValue(42));
    values.push_back(AnyValue(3.14));
    values.push_back(AnyValue(std::string("test")));
    values.push_back(AnyValue(true));
    
    EXPECT_EQ(values[0].get<int>(), 42);
    EXPECT_DOUBLE_EQ(values[1].get<double>(), 3.14);
    EXPECT_EQ(values[2].get<std::string>(), "test");
    EXPECT_TRUE(values[3].get<bool>());
}

// メモリリークがないことを確認するテスト
TEST_F(AnyValueTest, NoMemoryLeaks) {
    // 単純なRAII確認 - デストラクタが正しく呼ばれること
    {
        AnyValue value(std::string("temporary"));
        EXPECT_EQ(value.get<std::string>(), "temporary");
    } // ここでデストラクタが呼ばれる
    
    // 再代入でのメモリ管理
    AnyValue value(42);
    value = std::string("replaced");
    value = 3.14;
    value.reset();
    // すべて適切にクリーンアップされるはず
}