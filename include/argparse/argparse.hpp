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
        
        // TypeConverter: 文字列から各型への変換機能
        class TypeConverter {
        public:
            // 基本型変換器のテンプレート
            template<typename T>
            static std::function<AnyValue(const std::string&)> get_converter();
            
            // int変換
            static std::function<AnyValue(const std::string&)> int_converter() {
                return [](const std::string& value) -> AnyValue {
                    try {
                        // 先頭と末尾の空白を除去
                        std::string trimmed = trim(value);
                        if (trimmed.empty()) {
                            throw std::invalid_argument("empty string cannot be converted to int");
                        }
                        
                        size_t pos;
                        int result = std::stoi(trimmed, &pos);
                        
                        // 全体が変換されたかチェック
                        if (pos != trimmed.length()) {
                            throw std::invalid_argument("invalid int value: '" + value + "'");
                        }
                        
                        return AnyValue(result);
                    } catch (const std::invalid_argument&) {
                        throw std::invalid_argument("invalid int value: '" + value + "'");
                    } catch (const std::out_of_range&) {
                        throw std::invalid_argument("int value out of range: '" + value + "'");
                    }
                };
            }
            
            // float/double変換
            static std::function<AnyValue(const std::string&)> float_converter() {
                return [](const std::string& value) -> AnyValue {
                    try {
                        std::string trimmed = trim(value);
                        if (trimmed.empty()) {
                            throw std::invalid_argument("empty string cannot be converted to float");
                        }
                        
                        size_t pos;
                        double result = std::stod(trimmed, &pos);
                        
                        if (pos != trimmed.length()) {
                            throw std::invalid_argument("invalid float value: '" + value + "'");
                        }
                        
                        return AnyValue(result);
                    } catch (const std::invalid_argument&) {
                        throw std::invalid_argument("invalid float value: '" + value + "'");
                    } catch (const std::out_of_range&) {
                        throw std::invalid_argument("float value out of range: '" + value + "'");
                    }
                };
            }
            
            // bool変換
            static std::function<AnyValue(const std::string&)> bool_converter() {
                return [](const std::string& value) -> AnyValue {
                    std::string lower_value = to_lower(trim(value));
                    
                    if (lower_value.empty()) {
                        throw std::invalid_argument("empty string cannot be converted to bool");
                    }
                    
                    // true値
                    if (lower_value == "true" || lower_value == "1" || 
                        lower_value == "yes" || lower_value == "on") {
                        return AnyValue(true);
                    }
                    
                    // false値
                    if (lower_value == "false" || lower_value == "0" || 
                        lower_value == "no" || lower_value == "off") {
                        return AnyValue(false);
                    }
                    
                    throw std::invalid_argument("invalid bool value: '" + value + "' (expected: true/false, 1/0, yes/no, on/off)");
                };
            }
            
            // string変換（デフォルト）
            static std::function<AnyValue(const std::string&)> string_converter() {
                return [](const std::string& value) -> AnyValue {
                    return AnyValue(value);
                };
            }
            
            // 型名から変換器を取得
            static std::function<AnyValue(const std::string&)> get_converter_by_name(const std::string& type_name) {
                if (type_name == "int") {
                    return int_converter();
                } else if (type_name == "float" || type_name == "double") {
                    return float_converter();
                } else if (type_name == "bool") {
                    return bool_converter();
                } else if (type_name == "string" || type_name == "str") {
                    return string_converter();
                } else {
                    // 不明な型はデフォルトでstring変換
                    return string_converter();
                }
            }
            
            // カスタム変換器の作成ヘルパー
            template<typename T>
            static std::function<AnyValue(const std::string&)> create_custom_converter(
                std::function<T(const std::string&)> converter_func) {
                return [converter_func](const std::string& value) -> AnyValue {
                    try {
                        T result = converter_func(value);
                        return AnyValue(result);
                    } catch (const std::exception& e) {
                        throw std::invalid_argument("custom conversion failed for '" + value + "': " + e.what());
                    }
                };
            }
            
        private:
            // ユーティリティ関数
            static std::string trim(const std::string& str) {
                size_t start = str.find_first_not_of(" \t\n\r");
                if (start == std::string::npos) return "";
                
                size_t end = str.find_last_not_of(" \t\n\r");
                return str.substr(start, end - start + 1);
            }
            
            static std::string to_lower(const std::string& str) {
                std::string result = str;
                std::transform(result.begin(), result.end(), result.begin(), 
                    [](unsigned char c) { return std::tolower(c); });
                return result;
            }
        };
        
        // テンプレート特殊化によるget_converterの実装
        template<>
        std::function<AnyValue(const std::string&)> TypeConverter::get_converter<int>() {
            return int_converter();
        }
        
        template<>
        std::function<AnyValue(const std::string&)> TypeConverter::get_converter<float>() {
            return float_converter();
        }
        
        template<>
        std::function<AnyValue(const std::string&)> TypeConverter::get_converter<double>() {
            return float_converter();
        }
        
        template<>
        std::function<AnyValue(const std::string&)> TypeConverter::get_converter<bool>() {
            return bool_converter();
        }
        
        template<>
        std::function<AnyValue(const std::string&)> TypeConverter::get_converter<std::string>() {
            return string_converter();
        }
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
        
        // カスタム型変換器の設定
        template<typename T>
        Argument& converter(std::function<T(const std::string&)> converter_func) {
            definition_.converter = detail::TypeConverter::create_custom_converter<T>(converter_func);
            return *this;
        }
        
        // 直接的なconverter設定（上級者向け）
        Argument& converter(std::function<detail::AnyValue(const std::string&)> converter_func) {
            definition_.converter = converter_func;
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
        
        // Setup converter based on type using TypeConverter
        void _setup_converter_for_type(const std::string& type_name) {
            definition_.converter = detail::TypeConverter::get_converter_by_name(type_name);
        }
    };
    
    // ArgumentParser: メインパーサークラス
    class ArgumentParser {
    private:
        std::string prog_;
        std::string description_;
        std::string epilog_;
        std::string usage_;
        bool add_help_;
        
        std::vector<std::shared_ptr<Argument>> arguments_;
        std::map<std::string, std::shared_ptr<Argument>> argument_map_;
        
    public:
        // コンストラクタ
        explicit ArgumentParser(const std::string& prog = "", 
                               const std::string& description = "",
                               const std::string& epilog = "",
                               bool add_help = true)
            : prog_(prog), description_(description), epilog_(epilog), add_help_(add_help) {
            
            // prog が空の場合は後でargv[0]から設定される
            if (prog_.empty()) {
                prog_ = "program";  // デフォルト名、parse_argsで上書きされる
            }
        }
        
        // add_argumentメソッド - 位置引数版
        Argument& add_argument(const std::string& name) {
            auto arg = std::make_shared<Argument>(name);
            arguments_.push_back(arg);
            
            // 引数名をマップに登録（位置引数かオプション引数かに関わらず）
            argument_map_[name] = arg;
            
            return *arg;
        }
        
        // add_argumentメソッド - オプション引数版（短縮形と長形式）
        Argument& add_argument(const std::string& short_name, const std::string& long_name) {
            std::vector<std::string> names = {short_name, long_name};
            auto arg = std::make_shared<Argument>(names);
            arguments_.push_back(arg);
            
            // 両方の名前をマップに登録
            argument_map_[short_name] = arg;
            argument_map_[long_name] = arg;
            
            return *arg;
        }
        
        // add_argumentメソッド - 複数名前版
        Argument& add_argument(const std::vector<std::string>& names) {
            auto arg = std::make_shared<Argument>(names);
            arguments_.push_back(arg);
            
            // すべての名前をマップに登録
            for (const auto& name : names) {
                argument_map_[name] = arg;
            }
            
            return *arg;
        }
        
        // Getter methods
        const std::string& prog() const { return prog_; }
        const std::string& description() const { return description_; }
        const std::string& epilog() const { return epilog_; }
        
        // Set program name (usually called from parse_args with argv[0])
        void set_prog(const std::string& prog) {
            prog_ = prog;
        }
        
        // Get number of arguments
        size_t argument_count() const {
            return arguments_.size();
        }
        
        // Check if argument exists by name
        bool has_argument(const std::string& name) const {
            return argument_map_.find(name) != argument_map_.end();
        }
        
        // Get argument by name (for internal use)
        std::shared_ptr<Argument> get_argument(const std::string& name) const {
            auto it = argument_map_.find(name);
            if (it != argument_map_.end()) {
                return it->second;
            }
            return nullptr;
        }
        
        // Get all arguments (for internal use)
        const std::vector<std::shared_ptr<Argument>>& get_arguments() const {
            return arguments_;
        }
        
        // Check if this parser has help enabled
        bool help_enabled() const {
            return add_help_;
        }
        
    private:
        // Extract program name from path (removes directory path)
        std::string _extract_prog_name(const std::string& argv0) const {
            size_t last_slash = argv0.find_last_of("/\\");
            if (last_slash != std::string::npos) {
                return argv0.substr(last_slash + 1);
            }
            return argv0;
        }
    };
    
} // namespace argparse

#endif // ARGPARSE_HPP_INCLUDED