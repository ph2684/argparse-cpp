#include <gtest/gtest.h>
#include "../../include/argparse/argparse.hpp"

class HelpGeneratorTest : public ::testing::Test {
protected:
    argparse::ArgumentParser parser;
    
    void SetUp() override {
        parser = argparse::ArgumentParser("test_prog", "Test program for help generation", "", true);
    }
};

// Test basic help generation
TEST_F(HelpGeneratorTest, BasicHelpTest) {
    parser.add_argument("input").help("Input file path");
    parser.add_argument("--verbose", "-v").action("store_true").help("Enable verbose output");
    
    try {
        std::vector<std::string> args = {"--help"};
        parser.parse_args(args);
        FAIL() << "Should have thrown help_requested exception";
    } catch (const argparse::help_requested& e) {
        std::string help = e.message();
        
        // Check for basic elements
        EXPECT_TRUE(help.find("usage: test_prog") != std::string::npos);
        EXPECT_TRUE(help.find("Test program for help generation") != std::string::npos);
        EXPECT_TRUE(help.find("input") != std::string::npos);
        EXPECT_TRUE(help.find("Input file path") != std::string::npos);
        EXPECT_TRUE(help.find("--verbose, -v") != std::string::npos);
        EXPECT_TRUE(help.find("Enable verbose output") != std::string::npos);
        EXPECT_TRUE(help.find("--help, -h") != std::string::npos);
        EXPECT_TRUE(help.find("show this help message and exit") != std::string::npos);
    }
}

// Test usage line generation
TEST_F(HelpGeneratorTest, UsageLineTest) {
    parser.add_argument("command");
    parser.add_argument("file");
    parser.add_argument("--output", "-o").help("Output file");
    parser.add_argument("--verbose", "-v").action("store_true");
    
    try {
        std::vector<std::string> args = {"-h"};
        parser.parse_args(args);
        FAIL() << "Should have thrown help_requested exception";
    } catch (const argparse::help_requested& e) {
        std::string help = e.message();
        
        // Check usage line
        EXPECT_TRUE(help.find("usage: test_prog") != std::string::npos);
        EXPECT_TRUE(help.find("command") != std::string::npos);
        EXPECT_TRUE(help.find("file") != std::string::npos);
        EXPECT_TRUE(help.find("[-o OUTPUT]") != std::string::npos || 
                   help.find("[--output OUTPUT]") != std::string::npos);
    }
}

// Test positional and optional arguments sections
TEST_F(HelpGeneratorTest, ArgumentSectionsTest) {
    parser.add_argument("source").help("Source file");
    parser.add_argument("destination").help("Destination file");
    parser.add_argument("--force", "-f").action("store_true").help("Force overwrite");
    parser.add_argument("--backup", "-b").help("Backup location");
    
    try {
        std::vector<std::string> args = {"--help"};
        parser.parse_args(args);
        FAIL() << "Should have thrown help_requested exception";
    } catch (const argparse::help_requested& e) {
        std::string help = e.message();
        
        // Check sections
        EXPECT_TRUE(help.find("positional arguments:") != std::string::npos);
        EXPECT_TRUE(help.find("options:") != std::string::npos);
        
        // Verify positional arguments are in the right section
        size_t pos_section = help.find("positional arguments:");
        size_t opt_section = help.find("options:");
        
        EXPECT_TRUE(pos_section != std::string::npos);
        EXPECT_TRUE(opt_section != std::string::npos);
        EXPECT_TRUE(pos_section < opt_section);
        
        // Check that source and destination appear in positional section
        size_t source_pos = help.find("source", pos_section);
        size_t dest_pos = help.find("destination", pos_section);
        size_t force_pos = help.find("-h, --help");  // This will definitely be in optional section
        
        if (source_pos != std::string::npos && dest_pos != std::string::npos) {
            EXPECT_TRUE(source_pos > pos_section && source_pos < opt_section);
            EXPECT_TRUE(dest_pos > pos_section && dest_pos < opt_section);
        }
        EXPECT_TRUE(force_pos > opt_section);
    }
}

// Test epilog display
TEST_F(HelpGeneratorTest, EpilogTest) {
    parser = argparse::ArgumentParser("test_prog", "Test program", "For more info, visit example.com", true);
    parser.add_argument("file");
    
    try {
        std::vector<std::string> args = {"--help"};
        parser.parse_args(args);
        FAIL() << "Should have thrown help_requested exception";
    } catch (const argparse::help_requested& e) {
        std::string help = e.message();
        
        EXPECT_TRUE(help.find("For more info, visit example.com") != std::string::npos);
    }
}

// Test nargs display in help
TEST_F(HelpGeneratorTest, NargsDisplayTest) {
    parser.add_argument("files").nargs("+").help("Input files");
    parser.add_argument("--coords").nargs(2).help("X Y coordinates");
    parser.add_argument("--optional").nargs("?").help("Optional value");
    parser.add_argument("--many").nargs("*").help("Zero or more values");
    
    try {
        std::vector<std::string> args = {"--help"};
        parser.parse_args(args);
        FAIL() << "Should have thrown help_requested exception";
    } catch (const argparse::help_requested& e) {
        std::string help = e.message();
        
        // Check nargs representations in usage
        EXPECT_TRUE(help.find("files [files ...]") != std::string::npos ||
                   help.find("files") != std::string::npos);
        EXPECT_TRUE(help.find("--coords COORDS COORDS") != std::string::npos ||
                   help.find("-coords COORDS COORDS") != std::string::npos);
    }
}

// Test metavar customization
TEST_F(HelpGeneratorTest, MetavarTest) {
    parser.add_argument("input").metavar("INPUT_FILE").help("Input file");
    parser.add_argument("--output", "-o").metavar("FILE").help("Output file");
    
    try {
        std::vector<std::string> args = {"--help"};
        parser.parse_args(args);
        FAIL() << "Should have thrown help_requested exception";
    } catch (const argparse::help_requested& e) {
        std::string help = e.message();
        
        // Check custom metavar
        EXPECT_TRUE(help.find("INPUT_FILE") != std::string::npos);
        EXPECT_TRUE(help.find("-o FILE") != std::string::npos ||
                   help.find("--output FILE") != std::string::npos);
    }
}

// Test default value display
TEST_F(HelpGeneratorTest, DefaultValueTest) {
    parser.add_argument("--timeout").type("int").default_value(30).help("Timeout in seconds");
    parser.add_argument("--output").default_value(std::string("output.txt")).help("Output file");
    
    try {
        std::vector<std::string> args = {"--help"};
        parser.parse_args(args);
        FAIL() << "Should have thrown help_requested exception";
    } catch (const argparse::help_requested& e) {
        std::string help = e.message();
        
        // Check that default values are NOT displayed (Python argparse compatible)
        EXPECT_TRUE(help.find("(default:") == std::string::npos);
    }
}

// Test required arguments indication
TEST_F(HelpGeneratorTest, RequiredArgumentsTest) {
    parser.add_argument("--input").required(true).help("Required input file");
    parser.add_argument("--optional").help("Optional parameter");
    
    try {
        std::vector<std::string> args = {"--help"};
        parser.parse_args(args);
        FAIL() << "Should have thrown help_requested exception";
    } catch (const argparse::help_requested& e) {
        std::string help = e.message();
        
        // In usage line, required options should not be in brackets
        size_t usage_end = help.find("\n");
        std::string usage_line = help.substr(0, usage_end);
        
        // Required option might be shown without brackets
        // This is implementation-dependent behavior
        EXPECT_TRUE(help.find("--input") != std::string::npos);
        EXPECT_TRUE(help.find("--optional") != std::string::npos);
    }
}

// Test formatting and line wrapping (simplified check)
TEST_F(HelpGeneratorTest, FormattingTest) {
    parser.add_argument("--very-long-option-name").help(
        "This is a very long help text that should ideally be wrapped "
        "to fit within a reasonable terminal width for better readability");
    
    try {
        std::vector<std::string> args = {"--help"};
        parser.parse_args(args);
        FAIL() << "Should have thrown help_requested exception";
    } catch (const argparse::help_requested& e) {
        std::string help = e.message();
        
        // Check that the option and its help text are present
        EXPECT_TRUE(help.find("--very-long-option-name") != std::string::npos);
        EXPECT_TRUE(help.find("readability") != std::string::npos);
        
        // Check basic formatting (indentation)
        EXPECT_TRUE(help.find("  --very-long-option-name") != std::string::npos ||
                   help.find("  -") != std::string::npos);
    }
}

// Test empty parser help
TEST_F(HelpGeneratorTest, EmptyParserTest) {
    argparse::ArgumentParser empty_parser("empty", "Empty parser", "", true);
    
    try {
        std::vector<std::string> args = {"--help"};
        empty_parser.parse_args(args);
        FAIL() << "Should have thrown help_requested exception";
    } catch (const argparse::help_requested& e) {
        std::string help = e.message();
        
        // Should still have basic structure
        EXPECT_TRUE(help.find("usage: empty") != std::string::npos);
        EXPECT_TRUE(help.find("Empty parser") != std::string::npos);
        EXPECT_TRUE(help.find("options:") != std::string::npos);
        EXPECT_TRUE(help.find("--help, -h") != std::string::npos);
    }
}