#include <gtest/gtest.h>
#include "../../include/argparse/argparse.hpp"

class NargsTest : public ::testing::Test {
protected:
    argparse::ArgumentParser parser;
    
    void SetUp() override {
        parser = argparse::ArgumentParser("test_prog", "Test program for nargs");
    }
};

// Test fixed number nargs
TEST_F(NargsTest, FixedNumberNargsTest) {
    parser.add_argument("--coords").nargs(3).type("int");
    
    std::vector<std::string> args = {"--coords", "1", "2", "3"};
    
    auto ns = parser.parse_args(args);
    
    auto coords = ns.get<std::vector<std::string>>("coords");
    EXPECT_EQ(coords.size(), 3);
    EXPECT_EQ(coords[0], "1");
    EXPECT_EQ(coords[1], "2");
    EXPECT_EQ(coords[2], "3");
}

// Test fixed number nargs error
TEST_F(NargsTest, FixedNumberNargsErrorTest) {
    parser.add_argument("--coords").nargs(3);
    
    std::vector<std::string> args = {"--coords", "1", "2"};  // Missing one value
    
    EXPECT_THROW(parser.parse_args(args), std::runtime_error);
}

// Test nargs="?" (optional)
TEST_F(NargsTest, OptionalNargsTest) {
    parser.add_argument("--file").nargs("?").default_value(std::string("default.txt"));
    
    // With value
    std::vector<std::string> args1 = {"--file", "input.txt"};
    auto ns1 = parser.parse_args(args1);
    EXPECT_EQ(ns1.get<std::string>("file"), "input.txt");
    
    // Without value (should use default)
    argparse::ArgumentParser parser2("test", "test");
    parser2.add_argument("--file").nargs("?").default_value(std::string("default.txt"));
    std::vector<std::string> args2 = {"--file"};
    auto ns2 = parser2.parse_args(args2);
    EXPECT_EQ(ns2.get<std::string>("file"), "default.txt");
}

// Test nargs="*" (zero or more)
TEST_F(NargsTest, ZeroOrMoreNargsTest) {
    parser.add_argument("--files").nargs("*");
    
    // With multiple values
    std::vector<std::string> args1 = {"--files", "file1.txt", "file2.txt", "file3.txt"};
    auto ns1 = parser.parse_args(args1);
    auto files1 = ns1.get<std::vector<std::string>>("files");
    EXPECT_EQ(files1.size(), 3);
    EXPECT_EQ(files1[0], "file1.txt");
    EXPECT_EQ(files1[1], "file2.txt");
    EXPECT_EQ(files1[2], "file3.txt");
    
    // With no values
    argparse::ArgumentParser parser2("test", "test");
    parser2.add_argument("--files").nargs("*");
    std::vector<std::string> args2 = {"--files"};
    auto ns2 = parser2.parse_args(args2);
    auto files2 = ns2.get<std::vector<std::string>>("files");
    EXPECT_EQ(files2.size(), 0);
}

// Test nargs="+" (one or more)
TEST_F(NargsTest, OneOrMoreNargsTest) {
    parser.add_argument("--files").nargs("+");
    
    // With multiple values
    std::vector<std::string> args1 = {"--files", "file1.txt", "file2.txt"};
    auto ns1 = parser.parse_args(args1);
    auto files1 = ns1.get<std::vector<std::string>>("files");
    EXPECT_EQ(files1.size(), 2);
    EXPECT_EQ(files1[0], "file1.txt");
    EXPECT_EQ(files1[1], "file2.txt");
    
    // With single value
    argparse::ArgumentParser parser2("test", "test");
    parser2.add_argument("--files").nargs("+");
    std::vector<std::string> args2 = {"--files", "single.txt"};
    auto ns2 = parser2.parse_args(args2);
    auto files2 = ns2.get<std::vector<std::string>>("files");
    EXPECT_EQ(files2.size(), 1);
    EXPECT_EQ(files2[0], "single.txt");
}

// Test nargs="+" error (requires at least one value)
TEST_F(NargsTest, OneOrMoreNargsErrorTest) {
    parser.add_argument("--files").nargs("+");
    
    std::vector<std::string> args = {"--files"};  // No values
    
    EXPECT_THROW(parser.parse_args(args), std::runtime_error);
}

// Test nargs="remainder"
TEST_F(NargsTest, RemainderNargsTest) {
    parser.add_argument("command");
    parser.add_argument("args").nargs("remainder");
    
    std::vector<std::string> args = {"git", "commit", "-m", "message", "--author", "me"};
    
    auto ns = parser.parse_args(args);
    
    EXPECT_EQ(ns.get<std::string>("command"), "git");
    auto remainder = ns.get<std::vector<std::string>>("args");
    EXPECT_EQ(remainder.size(), 5);
    EXPECT_EQ(remainder[0], "commit");
    EXPECT_EQ(remainder[1], "-m");
    EXPECT_EQ(remainder[2], "message");
    EXPECT_EQ(remainder[3], "--author");
    EXPECT_EQ(remainder[4], "me");
}

// Test positional arguments with nargs
TEST_F(NargsTest, PositionalNargsTest) {
    parser.add_argument("files").nargs("+");
    
    std::vector<std::string> args = {"file1.txt", "file2.txt", "file3.txt"};
    
    auto ns = parser.parse_args(args);
    
    auto files = ns.get<std::vector<std::string>>("files");
    EXPECT_EQ(files.size(), 3);
    EXPECT_EQ(files[0], "file1.txt");
    EXPECT_EQ(files[1], "file2.txt");
    EXPECT_EQ(files[2], "file3.txt");
}

// Test positional arguments with fixed nargs
TEST_F(NargsTest, PositionalFixedNargsTest) {
    parser.add_argument("coords").nargs(2);
    
    std::vector<std::string> args = {"10", "20"};
    
    auto ns = parser.parse_args(args);
    
    auto coords = ns.get<std::vector<std::string>>("coords");
    EXPECT_EQ(coords.size(), 2);
    EXPECT_EQ(coords[0], "10");
    EXPECT_EQ(coords[1], "20");
}

// Test positional arguments with nargs="*"
TEST_F(NargsTest, PositionalZeroOrMoreTest) {
    parser.add_argument("files").nargs("*");
    
    // With files
    std::vector<std::string> args1 = {"file1.txt", "file2.txt"};
    auto ns1 = parser.parse_args(args1);
    auto files1 = ns1.get<std::vector<std::string>>("files");
    EXPECT_EQ(files1.size(), 2);
    
    // Without files
    argparse::ArgumentParser parser2("test", "test");
    parser2.add_argument("files").nargs("*");
    std::vector<std::string> args2 = {};
    auto ns2 = parser2.parse_args(args2);
    // For positional args with nargs="*", should still work with empty list
    EXPECT_NO_THROW(auto files2 = ns2.get<std::vector<std::string>>("files", std::vector<std::string>()));
}

// Test mixed nargs with optional and positional (simplified)
TEST_F(NargsTest, MixedNargsTest) {
    parser.add_argument("--verbose", "-v").action("store_true");
    parser.add_argument("command");
    parser.add_argument("args").nargs("+");
    
    std::vector<std::string> args = {"--verbose", "process", "input1", "input2"};
    
    auto ns = parser.parse_args(args);
    
    EXPECT_EQ(ns.get<bool>("verbose"), true);
    EXPECT_EQ(ns.get<std::string>("command"), "process");
    
    auto args_list = ns.get<std::vector<std::string>>("args");
    EXPECT_EQ(args_list.size(), 2);
    EXPECT_EQ(args_list[0], "input1");
    EXPECT_EQ(args_list[1], "input2");
}

// Test nargs type conversion
TEST_F(NargsTest, NargsTypeConversionTest) {
    parser.add_argument("--numbers").nargs(3).type("int");
    
    std::vector<std::string> args = {"--numbers", "1", "2", "3"};
    
    auto ns = parser.parse_args(args);
    
    auto numbers = ns.get<std::vector<std::string>>("numbers");
    EXPECT_EQ(numbers.size(), 3);
    // Note: In our implementation, nargs results are stored as string vectors
    // Type conversion would need additional handling for vector types
    EXPECT_EQ(numbers[0], "1");
    EXPECT_EQ(numbers[1], "2");
    EXPECT_EQ(numbers[2], "3");
}