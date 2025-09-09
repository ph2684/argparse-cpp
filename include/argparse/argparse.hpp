#pragma once

#ifndef ARGPARSE_HPP_INCLUDED
#define ARGPARSE_HPP_INCLUDED

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
#include <type_traits>

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
        class Parser;
        class Tokenizer;
        class TypeConverter;
        class HelpGenerator;
        
        // Type erasure implementation for C++11 (std::any alternative)
        class BaseHolder {
        public:
            virtual ~BaseHolder() {}
            virtual BaseHolder* clone() const = 0;
            virtual const std::type_info& type() const = 0;
        };
        
        template<typename T>
        class Holder : public BaseHolder {
        private:
            T value_;
            
        public:
            explicit Holder(const T& value) : value_(value) {}
            explicit Holder(T&& value) : value_(std::move(value)) {}
            
            virtual BaseHolder* clone() const override {
                return new Holder<T>(value_);
            }
            
            virtual const std::type_info& type() const override {
                return typeid(T);
            }
            
            T& get() { return value_; }
            const T& get() const { return value_; }
        };
        
        class AnyValue {
        private:
            BaseHolder* holder_;
            
        public:
            // Default constructor
            AnyValue() : holder_(nullptr) {}
            
            // Constructor from value
            template<typename T>
            AnyValue(const T& value, 
                     typename std::enable_if<!std::is_same<AnyValue, typename std::decay<T>::type>::value>::type* = nullptr) 
                : holder_(new Holder<typename std::decay<T>::type>(value)) {}
            
            template<typename T>
            AnyValue(T&& value,
                     typename std::enable_if<!std::is_same<AnyValue, typename std::decay<T>::type>::value>::type* = nullptr) 
                : holder_(new Holder<typename std::decay<T>::type>(std::forward<T>(value))) {}
            
            // Copy constructor
            AnyValue(const AnyValue& other) 
                : holder_(other.holder_ ? other.holder_->clone() : nullptr) {}
            
            // Move constructor
            AnyValue(AnyValue&& other) : holder_(other.holder_) {
                other.holder_ = nullptr;
            }
            
            // Destructor
            ~AnyValue() {
                delete holder_;
            }
            
            // Copy assignment
            AnyValue& operator=(const AnyValue& other) {
                if (this != &other) {
                    delete holder_;
                    holder_ = other.holder_ ? other.holder_->clone() : nullptr;
                }
                return *this;
            }
            
            // Move assignment
            AnyValue& operator=(AnyValue&& other) {
                if (this != &other) {
                    delete holder_;
                    holder_ = other.holder_;
                    other.holder_ = nullptr;
                }
                return *this;
            }
            
            // Assignment from value
            template<typename T>
            typename std::enable_if<!std::is_same<AnyValue, typename std::decay<T>::type>::value, AnyValue&>::type
            operator=(const T& value) {
                delete holder_;
                holder_ = new Holder<typename std::decay<T>::type>(value);
                return *this;
            }
            
            template<typename T>
            typename std::enable_if<!std::is_same<AnyValue, typename std::decay<T>::type>::value, AnyValue&>::type
            operator=(T&& value) {
                delete holder_;
                holder_ = new Holder<typename std::decay<T>::type>(std::forward<T>(value));
                return *this;
            }
            
            // Type-safe getter
            template<typename T>
            T& get() {
                if (!holder_) {
                    throw std::runtime_error("AnyValue is empty");
                }
                
                Holder<T>* typed_holder = dynamic_cast<Holder<T>*>(holder_);
                if (!typed_holder) {
                    throw std::runtime_error(
                        "Type mismatch: stored type is " + 
                        std::string(holder_->type().name()) + 
                        ", requested type is " + 
                        std::string(typeid(T).name())
                    );
                }
                
                return typed_holder->get();
            }
            
            template<typename T>
            const T& get() const {
                if (!holder_) {
                    throw std::runtime_error("AnyValue is empty");
                }
                
                const Holder<T>* typed_holder = dynamic_cast<const Holder<T>*>(holder_);
                if (!typed_holder) {
                    throw std::runtime_error(
                        "Type mismatch: stored type is " + 
                        std::string(holder_->type().name()) + 
                        ", requested type is " + 
                        std::string(typeid(T).name())
                    );
                }
                
                return typed_holder->get();
            }
            
            // Check if empty
            bool empty() const { return holder_ == nullptr; }
            
            // Get stored type
            const std::type_info& type() const {
                if (!holder_) {
                    return typeid(void);
                }
                return holder_->type();
            }
            
            // Reset to empty state
            void reset() {
                delete holder_;
                holder_ = nullptr;
            }
            
            // Swap with another AnyValue
            void swap(AnyValue& other) {
                std::swap(holder_, other.holder_);
            }
        };
    }
    
    // Argument definition structure
    struct ArgumentDefinition {
        std::vector<std::string> names;              // ["--verbose", "-v"]
        std::string help;                           // Help text
        std::string metavar;                        // Display name in help
        std::string action;                         // "store", "store_true", etc.
        std::string type_name;                      // "int", "float", "string"
        detail::AnyValue default_value;             // Default value (type-erased)
        std::vector<detail::AnyValue> choices;      // Valid choices
        int nargs;                                  // Number of arguments (-1 = any)
        bool required;                              // Required flag
        std::function<detail::AnyValue(const std::string&)> converter;
        std::function<bool(const detail::AnyValue&)> validator;
        
        ArgumentDefinition() 
            : action("store"), type_name("string"), nargs(1), required(false) {}
    };
    
    // Argument class for managing individual argument definitions
    class Argument {
    private:
        ArgumentDefinition definition_;
        
    public:
        // Constructor
        explicit Argument(const std::vector<std::string>& names = std::vector<std::string>()) {
            definition_.names = names;
            _setup_default_converter();
        }
        
        explicit Argument(const std::string& name) {
            definition_.names.push_back(name);
            _setup_default_converter();
        }
        
        // Fluent interface for setting properties
        Argument& help(const std::string& text) {
            definition_.help = text;
            return *this;
        }
        
        Argument& metavar(const std::string& name) {
            definition_.metavar = name;
            return *this;
        }
        
        Argument& action(const std::string& action_type) {
            definition_.action = action_type;
            return *this;
        }
        
        Argument& type(const std::string& type_name) {
            definition_.type_name = type_name;
            _setup_converter_for_type(type_name);
            return *this;
        }
        
        template<typename T>
        Argument& default_value(const T& value) {
            definition_.default_value = value;
            return *this;
        }
        
        template<typename T>
        Argument& choices(const std::vector<T>& choice_list) {
            definition_.choices.clear();
            for (const auto& choice : choice_list) {
                definition_.choices.push_back(detail::AnyValue(choice));
            }
            return *this;
        }
        
        Argument& nargs(int count) {
            definition_.nargs = count;
            return *this;
        }
        
        Argument& nargs(const std::string& spec) {
            if (spec == "?") {
                definition_.nargs = -2;  // Special value for optional
            } else if (spec == "*") {
                definition_.nargs = -3;  // Special value for zero or more
            } else if (spec == "+") {
                definition_.nargs = -4;  // Special value for one or more
            }
            return *this;
        }
        
        Argument& required(bool is_required = true) {
            definition_.required = is_required;
            return *this;
        }
        
        // Access to definition
        const ArgumentDefinition& definition() const {
            return definition_;
        }
        
        ArgumentDefinition& definition() {
            return definition_;
        }
        
        // Check if this is a positional argument
        bool is_positional() const {
            if (definition_.names.empty()) return false;
            const std::string& name = definition_.names[0];
            return !name.empty() && name[0] != '-';
        }
        
        // Get primary name
        std::string get_name() const {
            if (definition_.names.empty()) return "";
            return definition_.names[0];
        }
        
        // Get all names
        const std::vector<std::string>& get_names() const {
            return definition_.names;
        }
        
        // Convert string value using the configured converter
        detail::AnyValue convert_value(const std::string& str_value) const {
            if (definition_.converter) {
                return definition_.converter(str_value);
            }
            return detail::AnyValue(str_value);  // Default to string
        }
        
        // Validate value using the configured validator
        bool validate_value(const detail::AnyValue& value) const {
            // Check choices if specified
            if (!definition_.choices.empty()) {
                bool found = false;
                for (const auto& choice : definition_.choices) {
                    if (choice.type() == value.type()) {
                        // For now, simple type check - full comparison would require operator==
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    return false;
                }
            }
            
            // Use custom validator if provided
            if (definition_.validator) {
                return definition_.validator(value);
            }
            
            return true;
        }
        
    private:
        // Setup default string converter
        void _setup_default_converter() {
            definition_.converter = [](const std::string& value) -> detail::AnyValue {
                return detail::AnyValue(value);
            };
        }
        
        // Setup converter based on type
        void _setup_converter_for_type(const std::string& type_name) {
            if (type_name == "int") {
                definition_.converter = [](const std::string& value) -> detail::AnyValue {
                    try {
                        return detail::AnyValue(std::stoi(value));
                    } catch (const std::exception&) {
                        throw std::invalid_argument("invalid int value: '" + value + "'");
                    }
                };
            } else if (type_name == "float" || type_name == "double") {
                definition_.converter = [](const std::string& value) -> detail::AnyValue {
                    try {
                        return detail::AnyValue(std::stod(value));
                    } catch (const std::exception&) {
                        throw std::invalid_argument("invalid float value: '" + value + "'");
                    }
                };
            } else if (type_name == "bool") {
                definition_.converter = [](const std::string& value) -> detail::AnyValue {
                    std::string lower_value = value;
                    std::transform(lower_value.begin(), lower_value.end(), lower_value.begin(), ::tolower);
                    if (lower_value == "true" || lower_value == "1" || lower_value == "yes") {
                        return detail::AnyValue(true);
                    } else if (lower_value == "false" || lower_value == "0" || lower_value == "no") {
                        return detail::AnyValue(false);
                    } else {
                        throw std::invalid_argument("invalid bool value: '" + value + "'");
                    }
                };
            } else {
                // Default to string
                _setup_default_converter();
            }
        }
    };
    
} // namespace argparse

#endif // ARGPARSE_HPP_INCLUDED