#include <gtest/gtest.h>
#include "../../include/argparse/argparse.hpp"

class ActionTest : public ::testing::Test {
protected:
    argparse::ArgumentParser parser;
    
    void SetUp() override {
        parser = argparse::ArgumentParser("test_prog", "Test program for actions");
    }
};

// Test store action (default)
TEST_F(ActionTest, StoreActionTest) {
    parser.add_argument("--name").action("store");
    parser.add_argument("--count").action("store").type("int");
    
    std::vector<std::string> args = {"--name", "john", "--count", "42"};
    
    auto ns = parser.parse_args(args);
    
    EXPECT_EQ(ns.get<std::string>("name"), "john");
    EXPECT_EQ(ns.get<int>("count"), 42);
}

// Test store_true action
TEST_F(ActionTest, StoreTrueActionTest) {
    parser.add_argument("--verbose", "-v").action("store_true");
    parser.add_argument("--debug").action("store_true");
    
    std::vector<std::string> args = {"--verbose"};
    
    auto ns = parser.parse_args(args);
    
    EXPECT_EQ(ns.get<bool>("verbose"), true);
    EXPECT_EQ(ns.get<bool>("debug", false), false);  // Not specified
}

// Test store_false action
TEST_F(ActionTest, StoreFalseActionTest) {
    parser.add_argument("--quiet", "-q").action("store_false");
    parser.add_argument("--no-color").action("store_false");
    
    std::vector<std::string> args = {"--quiet"};
    
    auto ns = parser.parse_args(args);
    
    EXPECT_EQ(ns.get<bool>("quiet"), false);
    EXPECT_EQ(ns.get<bool>("no-color", true), true);  // Not specified, should remain default
}

// Test count action
TEST_F(ActionTest, CountActionTest) {
    parser.add_argument("--verbose", "-v").action("count");
    
    std::vector<std::string> args = {"-v", "-v", "--verbose"};
    
    auto ns = parser.parse_args(args);
    
    EXPECT_EQ(ns.get<int>("verbose"), 3);
}

// Test count action with no occurrences
TEST_F(ActionTest, CountActionZeroTest) {
    parser.add_argument("--verbose", "-v").action("count");
    
    std::vector<std::string> args = {};
    
    auto ns = parser.parse_args(args);
    
    EXPECT_EQ(ns.get<int>("verbose", 0), 0);  // Should be 0 when not specified
}

// Test append action
TEST_F(ActionTest, AppendActionTest) {
    parser.add_argument("--file", "-f").action("append");
    
    std::vector<std::string> args = {"--file", "file1.txt", "-f", "file2.txt", "--file", "file3.txt"};
    
    auto ns = parser.parse_args(args);
    
    auto files = ns.get<std::vector<std::string>>("file");
    EXPECT_EQ(files.size(), 3);
    EXPECT_EQ(files[0], "file1.txt");
    EXPECT_EQ(files[1], "file2.txt");
    EXPECT_EQ(files[2], "file3.txt");
}

// Test append action with single value
TEST_F(ActionTest, AppendActionSingleTest) {
    parser.add_argument("--file", "-f").action("append");
    
    std::vector<std::string> args = {"--file", "single.txt"};
    
    auto ns = parser.parse_args(args);
    
    auto files = ns.get<std::vector<std::string>>("file");
    EXPECT_EQ(files.size(), 1);
    EXPECT_EQ(files[0], "single.txt");
}

// Test custom action
TEST_F(ActionTest, CustomActionTest) {
    // Custom action that accumulates string lengths
    auto length_accumulator = [](const argparse::detail::AnyValue& current, const std::string& value) -> argparse::detail::AnyValue {
        int current_length = 0;
        if (!current.empty()) {
            current_length = current.get<int>();
        }
        return argparse::detail::AnyValue(current_length + static_cast<int>(value.length()));
    };
    
    parser.add_argument("--accumulate").custom_action(length_accumulator);
    
    std::vector<std::string> args = {"--accumulate", "hello", "--accumulate", "world"};
    
    auto ns = parser.parse_args(args);
    
    EXPECT_EQ(ns.get<int>("accumulate"), 10);  // "hello"(5) + "world"(5) = 10
}

// Test custom action with no initial value
TEST_F(ActionTest, CustomActionInitialTest) {
    // Custom action that counts non-empty values
    auto counter = [](const argparse::detail::AnyValue& current, const std::string& value) -> argparse::detail::AnyValue {
        int count = 0;
        if (!current.empty()) {
            count = current.get<int>();
        }
        return argparse::detail::AnyValue(count + (value.empty() ? 0 : 1));
    };
    
    parser.add_argument("--count-values").custom_action(counter);
    
    std::vector<std::string> args = {"--count-values", "a", "--count-values", "", "--count-values", "b"};
    
    auto ns = parser.parse_args(args);
    
    EXPECT_EQ(ns.get<int>("count-values"), 2);  // Two non-empty values
}

// Test mixed actions
TEST_F(ActionTest, MixedActionsTest) {
    parser.add_argument("--verbose", "-v").action("count");
    parser.add_argument("--file", "-f").action("append");
    parser.add_argument("--debug").action("store_true");
    parser.add_argument("--name").action("store");
    
    std::vector<std::string> args = {
        "--name", "test",
        "-v", "-v",
        "--file", "a.txt",
        "--debug",
        "-f", "b.txt"
    };
    
    auto ns = parser.parse_args(args);
    
    EXPECT_EQ(ns.get<std::string>("name"), "test");
    EXPECT_EQ(ns.get<int>("verbose"), 2);
    EXPECT_EQ(ns.get<bool>("debug"), true);
    
    auto files = ns.get<std::vector<std::string>>("file");
    EXPECT_EQ(files.size(), 2);
    EXPECT_EQ(files[0], "a.txt");
    EXPECT_EQ(files[1], "b.txt");
}

// Test action errors
TEST_F(ActionTest, AppendRequiresValueTest) {
    parser.add_argument("--file").action("append");
    
    std::vector<std::string> args = {"--file"};  // Missing value
    
    EXPECT_THROW(parser.parse_args(args), std::runtime_error);
}

TEST_F(ActionTest, StoreRequiresValueTest) {
    parser.add_argument("--name").action("store");
    
    std::vector<std::string> args = {"--name"};  // Missing value
    
    EXPECT_THROW(parser.parse_args(args), std::runtime_error);
}

// Test count action doesn't require value
TEST_F(ActionTest, CountNoValueTest) {
    parser.add_argument("--verbose").action("count");
    
    std::vector<std::string> args = {"--verbose"};
    
    EXPECT_NO_THROW(auto ns = parser.parse_args(args));
}

// Test store_true/false actions don't require values
TEST_F(ActionTest, BooleanActionsNoValueTest) {
    parser.add_argument("--flag1").action("store_true");
    parser.add_argument("--flag2").action("store_false");
    
    std::vector<std::string> args = {"--flag1", "--flag2"};
    
    EXPECT_NO_THROW(auto ns = parser.parse_args(args));
}