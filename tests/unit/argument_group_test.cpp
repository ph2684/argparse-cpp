#include <gtest/gtest.h>
#include "../../include/argparse/argparse.hpp"
#include <sstream>

using namespace argparse;

class ArgumentGroupTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Basic ArgumentGroup creation test
TEST_F(ArgumentGroupTest, BasicCreation) {
    ArgumentGroup group("test group", "A test group for validation");
    
    EXPECT_EQ(group.title(), "test group");
    EXPECT_EQ(group.description(), "A test group for validation");
    EXPECT_EQ(group.argument_count(), 0);
    EXPECT_TRUE(group.empty());
}

// Test adding arguments to a group
TEST_F(ArgumentGroupTest, AddArguments) {
    ArgumentGroup group("test group", "A test group");
    
    // Add positional argument
    Argument& pos_arg = group.add_argument("filename");
    pos_arg.help("Input filename");
    
    // Add option arguments
    Argument& opt_arg1 = group.add_argument("-v", "--verbose");
    opt_arg1.action("store_true").help("Enable verbose output");
    
    Argument& opt_arg2 = group.add_argument("-f", "--format");
    opt_arg2.default_value(std::string("json")).help("Output format");
    
    EXPECT_EQ(group.argument_count(), 3);
    EXPECT_FALSE(group.empty());
    
    // Check that arguments were added correctly
    const auto& args = group.arguments();
    EXPECT_EQ(args.size(), 3);
    
    // Check first argument (positional)
    EXPECT_EQ(args[0]->get_name(), "filename");
    EXPECT_TRUE(args[0]->is_positional());
    EXPECT_EQ(args[0]->definition().help, "Input filename");
    
    // Check second argument (option)
    EXPECT_EQ(args[1]->get_names().size(), 2);
    EXPECT_EQ(args[1]->get_names()[0], "-v");
    EXPECT_EQ(args[1]->get_names()[1], "--verbose");
    EXPECT_FALSE(args[1]->is_positional());
    EXPECT_EQ(args[1]->definition().action, "store_true");
    
    // Check third argument (option with default)
    EXPECT_EQ(args[2]->get_names().size(), 2);
    EXPECT_EQ(args[2]->definition().help, "Output format");
    EXPECT_FALSE(args[2]->definition().default_value.empty());
}

// Test finding arguments by name
TEST_F(ArgumentGroupTest, FindArgument) {
    ArgumentGroup group("test group");
    
    group.add_argument("input");
    group.add_argument("-o", "--output");
    
    // Find positional argument
    auto pos_arg = group.find_argument("input");
    ASSERT_NE(pos_arg, nullptr);
    EXPECT_EQ(pos_arg->get_name(), "input");
    
    // Find option argument by short name
    auto opt_arg1 = group.find_argument("-o");
    ASSERT_NE(opt_arg1, nullptr);
    EXPECT_EQ(opt_arg1->get_names()[0], "-o");
    
    // Find option argument by long name
    auto opt_arg2 = group.find_argument("--output");
    ASSERT_NE(opt_arg2, nullptr);
    EXPECT_EQ(opt_arg2->get_names()[1], "--output");
    
    // Both should be the same argument
    EXPECT_EQ(opt_arg1, opt_arg2);
    
    // Non-existent argument
    auto not_found = group.find_argument("nonexistent");
    EXPECT_EQ(not_found, nullptr);
}

// Test ArgumentParser with custom argument groups
TEST_F(ArgumentGroupTest, ArgumentParserWithGroups) {
    ArgumentParser parser("test_prog", "Test program with argument groups");
    
    // Create a custom group
    ArgumentGroup& file_group = parser.add_argument_group("file options", "File-related options");
    file_group.add_argument("-i", "--input").help("Input file");
    file_group.add_argument("-o", "--output").help("Output file");
    
    // Create another custom group
    ArgumentGroup& format_group = parser.add_argument_group("format options", "Format-related options");
    format_group.add_argument("-f", "--format").default_value(std::string("json")).help("Output format");
    format_group.add_argument("--pretty").action("store_true").help("Pretty print output");
    
    // Add some arguments to default groups
    parser.add_argument("input_file").help("Input file path");
    parser.add_argument("-v", "--verbose").action("store_true").help("Verbose output");
    
    // Check that groups were created correctly
    const auto& groups = parser.get_groups();
    EXPECT_EQ(groups.size(), 4); // positional, optional, file options, format options
    
    // Check positional group
    auto pos_group = parser.get_positional_group();
    ASSERT_NE(pos_group, nullptr);
    EXPECT_EQ(pos_group->title(), "positional arguments");
    EXPECT_EQ(pos_group->argument_count(), 1); // input_file
    
    // Check optional group
    auto opt_group = parser.get_optional_group();
    ASSERT_NE(opt_group, nullptr);
    EXPECT_EQ(opt_group->title(), "optional arguments");
    EXPECT_GE(opt_group->argument_count(), 2); // -v, --help (possibly more if help is enabled)
    
    // Check custom groups
    EXPECT_EQ(groups[2]->title(), "file options");
    EXPECT_EQ(groups[2]->description(), "File-related options");
    EXPECT_EQ(groups[2]->argument_count(), 2);
    
    EXPECT_EQ(groups[3]->title(), "format options");
    EXPECT_EQ(groups[3]->description(), "Format-related options");
    EXPECT_EQ(groups[3]->argument_count(), 2);
}

// Test help output with argument groups
TEST_F(ArgumentGroupTest, HelpOutputWithGroups) {
    // Redirect stdout to capture help output
    std::stringstream captured_output;
    std::streambuf* orig_cout = std::cout.rdbuf();
    std::cout.rdbuf(captured_output.rdbuf());
    
    ArgumentParser parser("test_prog", "Test program", "", false); // Disable auto help to avoid exit
    
    // Create custom groups
    ArgumentGroup& file_group = parser.add_argument_group("file options", "Options for file handling");
    file_group.add_argument("-i", "--input").help("Input file path");
    file_group.add_argument("-o", "--output").help("Output file path");
    
    ArgumentGroup& misc_group = parser.add_argument_group("miscellaneous");
    misc_group.add_argument("--debug").action("store_true").help("Enable debug mode");
    
    // Add positional argument
    parser.add_argument("filename").help("File to process");
    
    // Add help manually and generate help message
    parser.add_argument("-h", "--help").action("help").help("show this help message and exit");
    
    // Get help string directly from HelpGenerator
    std::string help_output = detail::HelpGenerator::generate_help(parser);
    
    // Restore stdout
    std::cout.rdbuf(orig_cout);
    
    // Check that help output contains group titles
    EXPECT_TRUE(help_output.find("positional arguments:") != std::string::npos);
    EXPECT_TRUE(help_output.find("optional arguments:") != std::string::npos);
    EXPECT_TRUE(help_output.find("file options:") != std::string::npos);
    EXPECT_TRUE(help_output.find("Options for file handling") != std::string::npos);
    EXPECT_TRUE(help_output.find("miscellaneous:") != std::string::npos);
    
    // Check that arguments appear under their respective groups
    EXPECT_TRUE(help_output.find("filename") != std::string::npos);
    EXPECT_TRUE(help_output.find("-i, --input") != std::string::npos);
    EXPECT_TRUE(help_output.find("-o, --output") != std::string::npos);
    EXPECT_TRUE(help_output.find("--debug") != std::string::npos);
}

// Test parsing with grouped arguments
TEST_F(ArgumentGroupTest, ParsingWithGroups) {
    ArgumentParser parser("test_prog");
    
    // Create custom groups
    ArgumentGroup& file_group = parser.add_argument_group("file options");
    file_group.add_argument("-i", "--input").help("Input file");
    file_group.add_argument("-o", "--output").help("Output file");
    
    // Add positional argument
    parser.add_argument("command").help("Command to execute");
    
    // Test parsing
    std::vector<std::string> args = {"run", "-i", "input.txt", "-o", "output.txt"};
    Namespace ns = parser.parse_args(args);
    
    EXPECT_EQ(ns.get<std::string>("command"), "run");
    EXPECT_EQ(ns.get<std::string>("input"), "input.txt");
    EXPECT_EQ(ns.get<std::string>("output"), "output.txt");
}

// Test empty group handling
TEST_F(ArgumentGroupTest, EmptyGroups) {
    ArgumentParser parser("test_prog");
    
    // Create empty group
    ArgumentGroup& empty_group = parser.add_argument_group("empty group", "This group has no arguments");
    
    // Add some arguments to other groups
    parser.add_argument("filename");
    parser.add_argument("-v", "--verbose").action("store_true");
    
    const auto& groups = parser.get_groups();
    
    // Check that empty group exists but has no arguments
    bool found_empty_group = false;
    for (const auto& group : groups) {
        if (group->title() == "empty group") {
            found_empty_group = true;
            EXPECT_TRUE(group->empty());
            EXPECT_EQ(group->argument_count(), 0);
            break;
        }
    }
    EXPECT_TRUE(found_empty_group);
    
    // Generate help and ensure empty groups don't appear
    std::string help_output = detail::HelpGenerator::generate_help(parser);
    
    // Empty groups should not appear in help output
    EXPECT_TRUE(help_output.find("empty group:") == std::string::npos);
}