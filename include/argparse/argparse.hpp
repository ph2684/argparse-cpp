#pragma once

/*
 * argparse-cpp: Header-only C++ library for parsing command-line arguments
 * 
 * A C++11 implementation inspired by Python's argparse module, providing
 * familiar API for C++ developers.
 * 
 * Version: 0.1.0
 * License: MIT
 */

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <typeinfo>
#include <functional>

// Version information
#define ARGPARSE_VERSION_MAJOR 0
#define ARGPARSE_VERSION_MINOR 1
#define ARGPARSE_VERSION_PATCH 0
#define ARGPARSE_VERSION "0.1.0"

namespace argparse {
    
    // Forward declarations
    class ArgumentParser;
    class Argument;
    class ArgumentGroup;
    class Namespace;
    
    namespace detail {
        // Internal implementation details
        class AnyValue;
        class Parser;
        class Tokenizer;
        class TypeConverter;
        class HelpGenerator;
    }
    
} // namespace argparse