#include <gtest/gtest.h>
#include "../../include/argparse/argparse.hpp"

class MetavarTest : public ::testing::Test {
protected:
    argparse::ArgumentParser parser;
    
    void SetUp() override {
        parser = argparse::ArgumentParser("test_prog", "Test program for metavar testing", "", true);
    }
};

// Test custom metavar setting for option argument
TEST_F(MetavarTest, CustomMetavarOptionTest) {
    parser.add_argument("--count", "-c").help("Number of items").metavar("N");
    
    try {
        std::vector<std::string> args = {"--help"};
        parser.parse_args(args);
        FAIL() << "Should have thrown help_requested exception";
    } catch (const argparse::help_requested& e) {
        std::string help = e.message();
        
        // Check that custom metavar "N" is used instead of default "COUNT"
        EXPECT_TRUE(help.find("--count, -c N") != std::string::npos);
        EXPECT_TRUE(help.find("NUMBER") == std::string::npos);
        EXPECT_TRUE(help.find("COUNT") == std::string::npos);
    }
}

// Test custom metavar setting for positional argument
TEST_F(MetavarTest, CustomMetavarPositionalTest) {
    parser.add_argument("input").help("Input file").metavar("FILE");
    
    try {
        std::vector<std::string> args = {"--help"};
        parser.parse_args(args);
        FAIL() << "Should have thrown help_requested exception";
    } catch (const argparse::help_requested& e) {
        std::string help = e.message();
        
        // Check that custom metavar "FILE" is used for positional argument
        EXPECT_TRUE(help.find("FILE") != std::string::npos);
        EXPECT_TRUE(help.find("Input file") != std::string::npos);
        // Should appear in usage line as well - check various possible formats
        EXPECT_TRUE(help.find("usage: test_prog") != std::string::npos);
        EXPECT_TRUE(help.find("FILE") != std::string::npos);  // FILE should appear somewhere in the help
    }
}

// Test default metavar generation for long option
TEST_F(MetavarTest, DefaultMetavarLongOptionTest) {
    parser.add_argument("--output-file").help("Output file path");
    
    try {
        std::vector<std::string> args = {"--help"};
        parser.parse_args(args);
        FAIL() << "Should have thrown help_requested exception";
    } catch (const argparse::help_requested& e) {
        std::string help = e.message();
        
        // Check that default metavar is generated and uppercased
        EXPECT_TRUE(help.find("--output-file OUTPUT-FILE") != std::string::npos ||
                   help.find("--output-file OUTPUT_FILE") != std::string::npos);
    }
}

// Test default metavar generation for short option
TEST_F(MetavarTest, DefaultMetavarShortOptionTest) {
    parser.add_argument("-v").help("Verbosity level");
    
    try {
        std::vector<std::string> args = {"--help"};
        parser.parse_args(args);
        FAIL() << "Should have thrown help_requested exception";
    } catch (const argparse::help_requested& e) {
        std::string help = e.message();
        
        // Check that default metavar is generated and uppercased
        EXPECT_TRUE(help.find("-v V") != std::string::npos);
    }
}

// Test metavar with store_true action (should not display metavar)
TEST_F(MetavarTest, MetavarWithStoreTrueTest) {
    parser.add_argument("--verbose", "-v").action("store_true").help("Enable verbose mode").metavar("IGNORED");
    
    try {
        std::vector<std::string> args = {"--help"};
        parser.parse_args(args);
        FAIL() << "Should have thrown help_requested exception";
    } catch (const argparse::help_requested& e) {
        std::string help = e.message();
        
        // metavar should not be displayed for store_true actions
        EXPECT_TRUE(help.find("--verbose, -v") != std::string::npos);
        EXPECT_TRUE(help.find("IGNORED") == std::string::npos);
        EXPECT_TRUE(help.find("Enable verbose mode") != std::string::npos);
    }
}

// Test metavar with store_false action (should not display metavar)
TEST_F(MetavarTest, MetavarWithStoreFalseTest) {
    parser.add_argument("--no-backup").action("store_false").help("Disable backup").metavar("IGNORED");
    
    try {
        std::vector<std::string> args = {"--help"};
        parser.parse_args(args);
        FAIL() << "Should have thrown help_requested exception";
    } catch (const argparse::help_requested& e) {
        std::string help = e.message();
        
        // metavar should not be displayed for store_false actions
        EXPECT_TRUE(help.find("--no-backup") != std::string::npos);
        EXPECT_TRUE(help.find("IGNORED") == std::string::npos);
    }
}

// Test metavar with count action (should not display metavar)
TEST_F(MetavarTest, MetavarWithCountTest) {
    parser.add_argument("--verbose", "-v").action("count").help("Increase verbosity").metavar("IGNORED");
    
    try {
        std::vector<std::string> args = {"--help"};
        parser.parse_args(args);
        FAIL() << "Should have thrown help_requested exception";
    } catch (const argparse::help_requested& e) {
        std::string help = e.message();
        
        // metavar should not be displayed for count actions
        EXPECT_TRUE(help.find("--verbose, -v") != std::string::npos);
        EXPECT_TRUE(help.find("IGNORED") == std::string::npos);
    }
}

// Test metavar in usage line with nargs="?"
TEST_F(MetavarTest, MetavarWithNargsOptionalTest) {
    parser.add_argument("--config", "-c").nargs("?").metavar("FILE").help("Configuration file");
    
    try {
        std::vector<std::string> args = {"--help"};
        parser.parse_args(args);
        FAIL() << "Should have thrown help_requested exception";
    } catch (const argparse::help_requested& e) {
        std::string help = e.message();
        
        // Check metavar appears in help line
        EXPECT_TRUE(help.find("--config, -c FILE") != std::string::npos ||
                   help.find("--config FILE") != std::string::npos);
        
        // Check metavar appears correctly in usage line with [FILE]
        EXPECT_TRUE(help.find("[FILE]") != std::string::npos);
    }
}

// Test metavar in usage line with nargs="*"
TEST_F(MetavarTest, MetavarWithNargsZeroOrMoreTest) {
    parser.add_argument("files").nargs("*").metavar("FILE").help("Input files");
    
    try {
        std::vector<std::string> args = {"--help"};
        parser.parse_args(args);
        FAIL() << "Should have thrown help_requested exception";
    } catch (const argparse::help_requested& e) {
        std::string help = e.message();
        
        // Check metavar appears in help
        EXPECT_TRUE(help.find("files") != std::string::npos || help.find("FILE") != std::string::npos);
        
        // Check metavar appears correctly in usage line with appropriate nargs notation
        EXPECT_TRUE(help.find("[FILE [FILE ...]]") != std::string::npos);
    }
}

// Test metavar in usage line with nargs="+"
TEST_F(MetavarTest, MetavarWithNargsOneOrMoreTest) {
    parser.add_argument("files").nargs("+").metavar("FILE").help("Input files");
    
    try {
        std::vector<std::string> args = {"--help"};
        parser.parse_args(args);
        FAIL() << "Should have thrown help_requested exception";
    } catch (const argparse::help_requested& e) {
        std::string help = e.message();
        
        // Check metavar appears correctly in usage line with appropriate nargs notation
        EXPECT_TRUE(help.find("FILE [FILE ...]") != std::string::npos);
    }
}

// Test uppercase conversion for default metavar
TEST_F(MetavarTest, UppercaseConversionTest) {
    parser.add_argument("--lowercase-option").help("Test lowercase option");
    parser.add_argument("--mixed-Case-Option").help("Test mixed case option");
    
    try {
        std::vector<std::string> args = {"--help"};
        parser.parse_args(args);
        FAIL() << "Should have thrown help_requested exception";
    } catch (const argparse::help_requested& e) {
        std::string help = e.message();
        
        // Check that default metavars are uppercased
        EXPECT_TRUE(help.find("LOWERCASE-OPTION") != std::string::npos ||
                   help.find("LOWERCASE_OPTION") != std::string::npos);
        EXPECT_TRUE(help.find("MIXED-CASE-OPTION") != std::string::npos ||
                   help.find("MIXED_CASE_OPTION") != std::string::npos);
    }
}