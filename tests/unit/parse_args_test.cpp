#include <gtest/gtest.h>
#include "../../include/argparse/argparse.hpp"

class ParseArgsTest : public ::testing::Test {
protected:
    argparse::ArgumentParser parser;
    
    void SetUp() override {
        parser = argparse::ArgumentParser("test_prog", "Test program");
    }
};

// Basic argc/argv version test
TEST_F(ParseArgsTest, BasicArgcArgvTest) {
    parser.add_argument("name").help("Name argument");
    parser.add_argument("--verbose", "-v").action("store_true").help("Verbose mode");
    
    const char* argv[] = {"test_prog", "john", "--verbose"};
    int argc = sizeof(argv) / sizeof(argv[0]);
    
    auto ns = parser.parse_args(argc, const_cast<char**>(argv));
    
    EXPECT_EQ(ns.get<std::string>("name"), "john");
    EXPECT_EQ(ns.get<bool>("verbose"), true);
}

// Basic vector<string> version test
TEST_F(ParseArgsTest, BasicVectorTest) {
    parser.add_argument("name").help("Name argument");
    parser.add_argument("--count", "-c").type("int").default_value(1);
    
    std::vector<std::string> args = {"alice", "--count", "5"};
    
    auto ns = parser.parse_args(args);
    
    EXPECT_EQ(ns.get<std::string>("name"), "alice");
    EXPECT_EQ(ns.get<int>("count"), 5);
}

// Test default values
TEST_F(ParseArgsTest, DefaultValuesTest) {
    parser.add_argument("--timeout").type("int").default_value(30);
    parser.add_argument("--debug").action("store_true");
    
    std::vector<std::string> args = {};  // Empty args
    
    auto ns = parser.parse_args(args);
    
    EXPECT_EQ(ns.get<int>("timeout"), 30);
    EXPECT_EQ(ns.get<bool>("debug", false), false);  // Not set, should get default
}

// Test required arguments error
TEST_F(ParseArgsTest, RequiredArgumentErrorTest) {
    parser.add_argument("name").required(true);
    parser.add_argument("--optional").default_value(std::string("default"));
    
    std::vector<std::string> args = {"--optional", "value"};  // Missing required 'name'
    
    EXPECT_THROW(parser.parse_args(args), std::runtime_error);
}

// Test unknown argument error
TEST_F(ParseArgsTest, UnknownArgumentErrorTest) {
    parser.add_argument("name");
    
    std::vector<std::string> args = {"john", "--unknown"};
    
    EXPECT_THROW(parser.parse_args(args), std::runtime_error);
}

// Test type conversion error
TEST_F(ParseArgsTest, TypeConversionErrorTest) {
    parser.add_argument("--count").type("int");
    
    std::vector<std::string> args = {"--count", "not_a_number"};
    
    EXPECT_THROW(parser.parse_args(args), std::invalid_argument);
}

// Test multiple positional arguments
TEST_F(ParseArgsTest, MultiplePositionalTest) {
    parser.add_argument("first");
    parser.add_argument("second");
    parser.add_argument("--flag").action("store_true");
    
    std::vector<std::string> args = {"value1", "value2", "--flag"};
    
    auto ns = parser.parse_args(args);
    
    EXPECT_EQ(ns.get<std::string>("first"), "value1");
    EXPECT_EQ(ns.get<std::string>("second"), "value2");
    EXPECT_EQ(ns.get<bool>("flag"), true);
}

// Test mixed short and long options
TEST_F(ParseArgsTest, MixedOptionsTest) {
    parser.add_argument("--verbose", "-v").action("store_true");
    parser.add_argument("--output", "-o").default_value(std::string("output.txt"));
    parser.add_argument("input");
    
    std::vector<std::string> args = {"input.txt", "-v", "--output", "result.txt"};
    
    auto ns = parser.parse_args(args);
    
    EXPECT_EQ(ns.get<std::string>("input"), "input.txt");
    EXPECT_EQ(ns.get<bool>("verbose"), true);
    EXPECT_EQ(ns.get<std::string>("output"), "result.txt");
}

// Test program name setting from argv[0]
TEST_F(ParseArgsTest, ProgramNameFromArgvTest) {
    // Create a parser with empty prog name to test argv[0] extraction
    argparse::ArgumentParser test_parser("", "Test program");
    const char* argv[] = {"/usr/bin/myprogram", "arg1"};
    int argc = sizeof(argv) / sizeof(argv[0]);
    
    test_parser.add_argument("arg");
    auto ns = test_parser.parse_args(argc, const_cast<char**>(argv));
    
    EXPECT_EQ(test_parser.prog(), "myprogram");
    EXPECT_EQ(ns.get<std::string>("arg"), "arg1");
}

// Test empty arguments
TEST_F(ParseArgsTest, EmptyArgumentsTest) {
    parser.add_argument("--optional").default_value(std::string("default_value"));
    
    std::vector<std::string> args = {};
    
    auto ns = parser.parse_args(args);
    
    EXPECT_EQ(ns.get<std::string>("optional"), "default_value");
}

// Test help handling (--help should throw help_requested exception)
TEST_F(ParseArgsTest, HelpHandlingTest) {
    parser.add_argument("name");
    
    std::vector<std::string> args = {"--help"};
    
    // Should throw help_requested exception with help message
    EXPECT_THROW(parser.parse_args(args), argparse::help_requested);
}

// Test help message content
TEST_F(ParseArgsTest, HelpMessageContentTest) {
    parser.add_argument("input_file").help("Input file path");
    parser.add_argument("--verbose", "-v").action("store_true").help("Enable verbose output");
    parser.add_argument("--output", "-o").help("Output file path");
    
    std::vector<std::string> args = {"--help"};
    
    try {
        parser.parse_args(args);
        FAIL() << "Expected help_requested exception";
    } catch (const argparse::help_requested& e) {
        std::string help_msg = e.message();
        
        // Check that help message contains expected elements
        EXPECT_TRUE(help_msg.find("usage:") != std::string::npos);
        EXPECT_TRUE(help_msg.find("test_prog") != std::string::npos);
        EXPECT_TRUE(help_msg.find("input_file") != std::string::npos);
        EXPECT_TRUE(help_msg.find("--verbose") != std::string::npos);
        EXPECT_TRUE(help_msg.find("-v") != std::string::npos);
        EXPECT_TRUE(help_msg.find("--output") != std::string::npos);
        EXPECT_TRUE(help_msg.find("Enable verbose output") != std::string::npos);
    }
}