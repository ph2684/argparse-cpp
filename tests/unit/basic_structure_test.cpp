#include <gtest/gtest.h>
#include "../../include/argparse/argparse.hpp"

class BasicStructureTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    
    void TearDown() override {
    }
};

// バージョンマクロのテスト
TEST_F(BasicStructureTest, VersionMacros) {
    EXPECT_EQ(ARGPARSE_VERSION_MAJOR, 0);
    EXPECT_EQ(ARGPARSE_VERSION_MINOR, 1);
    EXPECT_EQ(ARGPARSE_VERSION_PATCH, 0);
    EXPECT_STREQ(ARGPARSE_VERSION, "0.1.0");
}

// ヘッダーガードのテスト（コンパイル時に検証）
TEST_F(BasicStructureTest, HeaderGuard) {
    // ARGPARSE_HPP_INCLUDEDマクロが定義されていることを確認
    #ifdef ARGPARSE_HPP_INCLUDED
        SUCCEED();
    #else
        FAIL() << "ARGPARSE_HPP_INCLUDED is not defined";
    #endif
}

// ネームスペースの基本テスト
TEST_F(BasicStructureTest, Namespaces) {
    // 実装済みのクラスが利用可能であることを確認
    argparse::Argument arg;
    
    // detail名前空間のクラスが利用可能であることを確認
    argparse::detail::AnyValue value;
    
    SUCCEED(); // コンパイルが通れば成功
}

// Forward declarations のテスト
TEST_F(BasicStructureTest, ForwardDeclarations) {
    // 実装済みクラスがインスタンス化できることを確認
    argparse::Argument* arg = new argparse::Argument();
    
    // メモリリークテストのためにdeleteも確認
    delete arg;
    
    SUCCEED();
}

// 基本的なコンパイルテスト
TEST_F(BasicStructureTest, BasicCompilation) {
    // 実装済みクラスが正常にインスタンス化できることを確認
    argparse::Argument arg("--test");
    argparse::detail::AnyValue value(42);
    
    // 基本的なメソッド呼び出しが可能であることを確認
    EXPECT_EQ(value.get<int>(), 42);
    EXPECT_EQ(arg.get_name(), "--test");
    EXPECT_FALSE(arg.is_positional());
}

// 標準ライブラリの依存関係テスト
TEST_F(BasicStructureTest, StandardLibraryIncludes) {
    // string
    std::string test_str = "test";
    EXPECT_EQ(test_str, "test");
    
    // vector
    std::vector<int> test_vec = {1, 2, 3};
    EXPECT_EQ(test_vec.size(), 3);
    
    // map
    std::map<std::string, int> test_map;
    test_map["key"] = 42;
    EXPECT_EQ(test_map["key"], 42);
    
    // memory (smart pointers)
    std::unique_ptr<int> test_ptr(new int(42));
    EXPECT_EQ(*test_ptr, 42);
    
    // functional
    std::function<int(int)> func = [](int x) { return x * 2; };
    EXPECT_EQ(func(21), 42);
    
    // algorithm
    std::vector<int> vec = {3, 1, 4, 1, 5};
    std::sort(vec.begin(), vec.end());
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[4], 5);
}

// 例外クラスの可用性テスト
TEST_F(BasicStructureTest, ExceptionClasses) {
    // std::runtime_error
    try {
        throw std::runtime_error("test error");
    } catch (const std::runtime_error& e) {
        EXPECT_STREQ(e.what(), "test error");
    }
    
    // std::invalid_argument
    try {
        throw std::invalid_argument("invalid arg");
    } catch (const std::invalid_argument& e) {
        EXPECT_STREQ(e.what(), "invalid arg");
    }
}

// type_info の可用性テスト
TEST_F(BasicStructureTest, TypeInfo) {
    int value = 42;
    const std::type_info& ti = typeid(value);
    
    EXPECT_EQ(ti, typeid(int));
    EXPECT_NE(ti, typeid(double));
    EXPECT_NE(ti, typeid(std::string));
}

// ヘッダオンリーライブラリの特性テスト
TEST_F(BasicStructureTest, HeaderOnlyLibrary) {
    // すべての実装がヘッダファイルに含まれていることを確認
    // （これはコンパイル時にリンクエラーが発生しないことで検証される）
    
    // 複数のクラスを組み合わせて使用
    argparse::Argument arg("--verbose");
    arg.help("Enable verbose output")
       .action("store_true")
       .default_value(false);
    
    // TypeConverterを直接使用
    auto converter = argparse::detail::TypeConverter::bool_converter();
    auto result = converter("true");
    
    EXPECT_TRUE(result.get<bool>());
}

// C++11 compatibility のテスト
TEST_F(BasicStructureTest, Cpp11Compatibility) {
    // auto keyword
    auto value = argparse::detail::AnyValue(42);
    EXPECT_EQ(value.get<int>(), 42);
    
    // nullptr
    argparse::detail::BaseHolder* holder = nullptr;
    EXPECT_EQ(holder, nullptr);
    
    // range-based for loop
    std::vector<int> numbers = {1, 2, 3};
    int sum = 0;
    for (auto num : numbers) {
        sum += num;
    }
    EXPECT_EQ(sum, 6);
    
    // lambda expressions
    auto lambda = [](int x) { return x * 2; };
    EXPECT_EQ(lambda(21), 42);
    
    // uniform initialization
    std::vector<std::string> names{"alice", "bob", "charlie"};
    EXPECT_EQ(names.size(), 3);
    
    // std::function
    std::function<bool(const std::string&)> validator = [](const std::string& s) {
        return !s.empty();
    };
    EXPECT_TRUE(validator("test"));
    EXPECT_FALSE(validator(""));
}

// RAII (Resource Acquisition Is Initialization) のテスト
TEST_F(BasicStructureTest, RAIICompliance) {
    // AnyValueのRAII確認
    {
        argparse::detail::AnyValue value(std::string("temporary"));
        EXPECT_EQ(value.get<std::string>(), "temporary");
    } // デストラクタが自動的に呼ばれる
    
    // Argumentクラスのexception safety
    try {
        argparse::Argument arg("--test");
        arg.type<int>();
        // 何らかの処理...
        throw std::runtime_error("test exception");
    } catch (const std::runtime_error&) {
        // 例外が発生してもリソースリークしない
    }
    
    SUCCEED(); // RAIIが適切に動作していればここに到達
}

// メモリ管理のテスト
TEST_F(BasicStructureTest, MemoryManagement) {
    // AnyValueでの動的メモリ管理
    argparse::detail::AnyValue value1;
    EXPECT_TRUE(value1.empty());
    
    value1 = std::string("test");
    EXPECT_FALSE(value1.empty());
    
    value1 = 42;  // 前の値は適切に解放される
    EXPECT_EQ(value1.get<int>(), 42);
    
    value1.reset();  // 明示的な解放
    EXPECT_TRUE(value1.empty());
    
    // コピー時のメモリ管理
    argparse::detail::AnyValue value2(std::string("original"));
    argparse::detail::AnyValue value3 = value2;  // Deep copy
    
    value2 = std::string("modified");
    EXPECT_EQ(value3.get<std::string>(), "original");  // 影響されない
    EXPECT_EQ(value2.get<std::string>(), "modified");
}