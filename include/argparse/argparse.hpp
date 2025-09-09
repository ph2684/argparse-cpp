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
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <typeinfo>
#include <functional>
#include <type_traits>
#include <iomanip>
#include <cctype>

// Version information
#define ARGPARSE_VERSION_MAJOR 0
#define ARGPARSE_VERSION_MINOR 1
#define ARGPARSE_VERSION_PATCH 0
#define ARGPARSE_VERSION "0.1.0"

namespace argparse {
    
    // Custom exception for help requests
    class help_requested : public std::exception {
    private:
        std::string help_message_;
        
    public:
        explicit help_requested(const std::string& message) : help_message_(message) {}
        
        virtual const char* what() const noexcept override {
            return help_message_.c_str();
        }
        
        const std::string& message() const {
            return help_message_;
        }
    };
    
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
        
        // Token structure for parsed arguments
        struct Token {
            enum Type {
                POSITIONAL,    // Non-option argument
                SHORT_OPTION,  // -o
                LONG_OPTION,   // --option
                OPTION_VALUE,  // Value for an option
                END_OPTIONS    // -- (end of options marker)
            };
            
            Type type;
            std::string value;
            std::string raw_value;  // Original value before processing
            
            Token(Type t, const std::string& val, const std::string& raw_val = "")
                : type(t), value(val), raw_value(raw_val.empty() ? val : raw_val) {}
        };
        
        // Tokenizer: コマンドライン引数のトークン化
        class Tokenizer {
        private:
            std::vector<Token> tokens_;
            size_t current_index_;
            bool end_of_options_;
            
        public:
            // Constructor
            Tokenizer() : current_index_(0), end_of_options_(false) {}
            
            // argv配列からトークンを生成
            void tokenize(int argc, char* argv[]) {
                std::vector<std::string> args;
                for (int i = 1; i < argc; ++i) {  // Skip program name (argv[0])
                    args.push_back(std::string(argv[i]));
                }
                tokenize(args);
            }
            
            // string配列からトークンを生成
            void tokenize(const std::vector<std::string>& args) {
                tokens_.clear();
                current_index_ = 0;
                end_of_options_ = false;
                
                for (size_t i = 0; i < args.size(); ++i) {
                    const std::string& arg = args[i];
                    
                    // "--" は引数終了マーカー
                    if (arg == "--" && !end_of_options_) {
                        tokens_.push_back(Token(Token::END_OPTIONS, arg));
                        end_of_options_ = true;
                        continue;
                    }
                    
                    // 引数終了後はすべて位置引数として扱う
                    if (end_of_options_) {
                        tokens_.push_back(Token(Token::POSITIONAL, arg));
                        continue;
                    }
                    
                    // オプション引数の判定と処理
                    if (arg.length() >= 2 && arg[0] == '-') {
                        if (arg[1] == '-') {
                            // 長形式オプション (--option or --option=value)
                            _process_long_option(arg);
                        } else {
                            // 短縮形オプション (-o or -o value or -abc)
                            _process_short_option(arg);
                        }
                    } else {
                        // 位置引数
                        tokens_.push_back(Token(Token::POSITIONAL, arg));
                    }
                }
            }
            
            // 次のトークンを取得
            Token next() {
                if (current_index_ >= tokens_.size()) {
                    throw std::runtime_error("No more tokens available");
                }
                return tokens_[current_index_++];
            }
            
            // 現在のトークンを確認（消費しない）
            Token peek() const {
                if (current_index_ >= tokens_.size()) {
                    throw std::runtime_error("No more tokens available");
                }
                return tokens_[current_index_];
            }
            
            // トークンが残っているかチェック
            bool has_next() const {
                return current_index_ < tokens_.size();
            }
            
            // インデックスをリセット
            void reset() {
                current_index_ = 0;
            }
            
            // 全トークンの取得（デバッグ用）
            const std::vector<Token>& get_tokens() const {
                return tokens_;
            }
            
            // トークン数の取得
            size_t size() const {
                return tokens_.size();
            }
            
            // 現在位置の取得
            size_t position() const {
                return current_index_;
            }
            
            // 指定位置にシーク
            void seek(size_t pos) {
                if (pos > tokens_.size()) {
                    pos = tokens_.size();
                }
                current_index_ = pos;
            }
            
        private:
            // 長形式オプションの処理 (--option or --option=value)
            void _process_long_option(const std::string& arg) {
                size_t equals_pos = arg.find('=');
                
                if (equals_pos != std::string::npos) {
                    // --option=value 形式
                    std::string option = arg.substr(0, equals_pos);
                    std::string value = arg.substr(equals_pos + 1);
                    
                    // 引用符の処理
                    value = _unquote_string(value);
                    
                    tokens_.push_back(Token(Token::LONG_OPTION, option, arg));
                    tokens_.push_back(Token(Token::OPTION_VALUE, value, arg));
                } else {
                    // --option 形式
                    tokens_.push_back(Token(Token::LONG_OPTION, arg));
                }
            }
            
            // 短縮形オプションの処理 (-o or -abc)
            void _process_short_option(const std::string& arg) {
                if (arg.length() == 2) {
                    // 単一の短縮形オプション (-o)
                    tokens_.push_back(Token(Token::SHORT_OPTION, arg));
                } else {
                    // 複数の短縮形オプション (-abc → -a -b -c)
                    for (size_t i = 1; i < arg.length(); ++i) {
                        std::string single_opt = "-" + std::string(1, arg[i]);
                        tokens_.push_back(Token(Token::SHORT_OPTION, single_opt, arg));
                    }
                }
            }
            
            // 引用符付き文字列の処理
            std::string _unquote_string(const std::string& str) {
                if (str.length() < 2) {
                    return str;
                }
                
                char first = str[0];
                char last = str[str.length() - 1];
                
                // ダブルクォートまたはシングルクォートで囲まれている場合
                if ((first == '"' && last == '"') || (first == '\'' && last == '\'')) {
                    std::string result = str.substr(1, str.length() - 2);
                    
                    // エスケープシーケンスの処理
                    if (first == '"') {
                        result = _process_escape_sequences(result);
                    }
                    
                    return result;
                }
                
                return str;
            }
            
            // エスケープシーケンスの処理
            std::string _process_escape_sequences(const std::string& str) {
                std::string result;
                result.reserve(str.length());
                
                for (size_t i = 0; i < str.length(); ++i) {
                    if (str[i] == '\\' && i + 1 < str.length()) {
                        char next = str[i + 1];
                        switch (next) {
                            case 'n': result += '\n'; break;
                            case 't': result += '\t'; break;
                            case 'r': result += '\r'; break;
                            case '\\': result += '\\'; break;
                            case '"': result += '"'; break;
                            case '\'': result += '\''; break;
                            default:
                                // 不明なエスケープシーケンスはそのまま保持
                                result += '\\';
                                result += next;
                                break;
                        }
                        ++i;  // 次の文字をスキップ
                    } else {
                        result += str[i];
                    }
                }
                
                return result;
            }
        };
        
        // HelpGenerator: ヘルプメッセージの生成
        class HelpGenerator {
        public:
            // ヘルプメッセージを生成（実装はArgumentParserクラス定義後）
            static std::string generate_help(const ArgumentParser& parser);
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
        std::function<detail::AnyValue(const detail::AnyValue&, const std::string&)> custom_action;  // Custom action handler
        
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
            } else if (spec == "remainder") {
                definition_.nargs = -5;  // Special value for remainder
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
        
        // カスタムアクション設定
        Argument& custom_action(std::function<detail::AnyValue(const detail::AnyValue&, const std::string&)> action_func) {
            definition_.custom_action = action_func;
            definition_.action = "custom";
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
                               bool add_help = false)
            : prog_(prog.empty() ? "program" : prog), description_(description), epilog_(epilog), add_help_(add_help) {
            
            // prog が空の場合は"program"をデフォルトとして設定
            // parse_argsでargv[0]から上書きされる場合がある
            
            // --help オプションを自動追加
            if (add_help_) {
                _add_help_argument();
            }
        }
        
        // add_argumentメソッド - 位置引数版
        Argument& add_argument(const std::string& name) {
            std::vector<std::string> names = {name};
            _validate_argument_names(names);
            
            auto arg = std::make_shared<Argument>(name);
            arguments_.push_back(arg);
            
            // 引数名をマップに登録（位置引数かオプション引数かに関わらず）
            argument_map_[name] = arg;
            
            return *arg;
        }
        
        // add_argumentメソッド - オプション引数版（短縮形と長形式）
        Argument& add_argument(const std::string& short_name, const std::string& long_name) {
            std::vector<std::string> names = {short_name, long_name};
            _validate_argument_names(names);
            
            auto arg = std::make_shared<Argument>(names);
            arguments_.push_back(arg);
            
            // 両方の名前をマップに登録
            argument_map_[short_name] = arg;
            argument_map_[long_name] = arg;
            
            return *arg;
        }
        
        // add_argumentメソッド - 複数名前版
        Argument& add_argument(const std::vector<std::string>& names) {
            _validate_argument_names(names);
            
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
        
        // Parse command line arguments (declaration only, implementation after detail::Parser)
        Namespace parse_args(int argc, char* argv[]);
        
        // Parse from string vector (declaration only, implementation after detail::Parser)
        Namespace parse_args(const std::vector<std::string>& args);
        
    private:
        // Extract program name from path (removes directory path)
        std::string _extract_prog_name(const std::string& argv0) const {
            size_t last_slash = argv0.find_last_of("/\\");
            if (last_slash != std::string::npos) {
                return argv0.substr(last_slash + 1);
            }
            return argv0;
        }
        
        // Validate argument name according to POSIX conventions
        bool _is_valid_argument_name(const std::string& name) const {
            if (name.empty()) {
                return false;  // 空の名前は不正
            }
            
            if (name[0] == '-') {
                // オプション引数の場合
                if (name.length() == 1) {
                    return false;  // "-" のみは不正
                }
                
                if (name[1] == '-') {
                    // 長形式 (--option)
                    if (name.length() == 2) {
                        return false;  // "--" のみは不正
                    }
                    
                    // 長形式は3文字以上で、英数字・ハイフン・アンダースコアのみ
                    for (size_t i = 2; i < name.length(); ++i) {
                        char c = name[i];
                        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || 
                              (c >= '0' && c <= '9') || c == '-' || c == '_')) {
                            return false;
                        }
                    }
                } else {
                    // 短縮形 (-o)
                    // 短縮形は通常1文字だが、複数文字も許可（-abc）
                    for (size_t i = 1; i < name.length(); ++i) {
                        char c = name[i];
                        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || 
                              (c >= '0' && c <= '9'))) {
                            return false;
                        }
                    }
                }
            } else {
                // 位置引数の場合
                // 英数字・アンダースコア・ハイフンのみ（先頭は英字またはアンダースコア）
                if (!((name[0] >= 'a' && name[0] <= 'z') || (name[0] >= 'A' && name[0] <= 'Z') || 
                      name[0] == '_')) {
                    return false;
                }
                
                for (size_t i = 1; i < name.length(); ++i) {
                    char c = name[i];
                    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || 
                          (c >= '0' && c <= '9') || c == '_' || c == '-')) {
                        return false;
                    }
                }
            }
            
            return true;
        }
        
        // Check for duplicate argument names
        void _check_duplicate_argument(const std::string& name) const {
            if (argument_map_.find(name) != argument_map_.end()) {
                throw std::runtime_error("Duplicate argument name: '" + name + "'");
            }
        }
        
        // Validate and check all names for an argument
        void _validate_argument_names(const std::vector<std::string>& names) const {
            if (names.empty()) {
                throw std::invalid_argument("Argument must have at least one name");
            }
            
            for (const auto& name : names) {
                if (!_is_valid_argument_name(name)) {
                    throw std::invalid_argument("Invalid argument name: '" + name + "'");
                }
                _check_duplicate_argument(name);
            }
        }
        
        // Add default --help argument
        void _add_help_argument() {
            // Check if --help or -h is already added by user
            if (argument_map_.find("--help") != argument_map_.end() || 
                argument_map_.find("-h") != argument_map_.end()) {
                return; // User has already defined help argument
            }
            
            std::vector<std::string> help_names = {"--help", "-h"};
            auto help_arg = std::make_shared<Argument>(help_names);
            help_arg->action("help")
                    .help("show this help message and exit");
            
            arguments_.push_back(help_arg);
            argument_map_["--help"] = help_arg;
            argument_map_["-h"] = help_arg;
        }
    };
    
    // Namespace class for storing parsed argument values
    class Namespace {
    private:
        std::unordered_map<std::string, detail::AnyValue> values_;
        
    public:
        // Default constructor
        Namespace() = default;
        
        // Copy constructor
        Namespace(const Namespace& other) : values_(other.values_) {}
        
        // Move constructor
        Namespace(Namespace&& other) : values_(std::move(other.values_)) {}
        
        // Copy assignment
        Namespace& operator=(const Namespace& other) {
            if (this != &other) {
                values_ = other.values_;
            }
            return *this;
        }
        
        // Move assignment
        Namespace& operator=(Namespace&& other) {
            if (this != &other) {
                values_ = std::move(other.values_);
            }
            return *this;
        }
        
        // Set a value by name
        template<typename T>
        void set(const std::string& name, const T& value) {
            values_[name] = detail::AnyValue(value);
        }
        
        // Set a value by name (move version)
        template<typename T>
        void set(const std::string& name, T&& value) {
            values_[name] = detail::AnyValue(std::forward<T>(value));
        }
        
        // Get a value by name with type safety
        template<typename T>
        T get(const std::string& name) const {
            auto it = values_.find(name);
            if (it == values_.end()) {
                throw std::runtime_error("Argument '" + name + "' not found");
            }
            return it->second.template get<T>();
        }
        
        // Get a value by name with default value
        template<typename T>
        T get(const std::string& name, const T& default_value) const {
            auto it = values_.find(name);
            if (it == values_.end()) {
                return default_value;
            }
            return it->second.template get<T>();
        }
        
        // Check if a value exists
        bool has(const std::string& name) const {
            return values_.find(name) != values_.end();
        }
        
        // Alias for has() for compatibility
        bool contains(const std::string& name) const {
            return has(name);
        }
        
        // Get all argument names
        std::vector<std::string> keys() const {
            std::vector<std::string> result;
            result.reserve(values_.size());
            for (const auto& pair : values_) {
                result.push_back(pair.first);
            }
            return result;
        }
        
        // Get number of stored values
        size_t size() const {
            return values_.size();
        }
        
        // Check if empty
        bool empty() const {
            return values_.empty();
        }
        
        // Clear all values
        void clear() {
            values_.clear();
        }
        
        // Remove a specific value
        bool remove(const std::string& name) {
            return values_.erase(name) > 0;
        }
        
        // Access to raw value (for advanced use)
        const detail::AnyValue& get_raw(const std::string& name) const {
            auto it = values_.find(name);
            if (it == values_.end()) {
                throw std::runtime_error("Argument '" + name + "' not found");
            }
            return it->second;
        }
        
        // Set raw value (for advanced use)
        void set_raw(const std::string& name, const detail::AnyValue& value) {
            values_[name] = value;
        }
        
        // Set raw value (move version)
        void set_raw(const std::string& name, detail::AnyValue&& value) {
            values_[name] = std::move(value);
        }
    };
    
    namespace detail {
        // Parser: コマンドライン引数の解析ロジック
        class Parser {
        private:
            Tokenizer tokenizer_;
            std::vector<std::shared_ptr<Argument>> positional_args_;
            std::map<std::string, std::shared_ptr<Argument>> option_args_;
            
        public:
            // Constructor
            Parser() = default;
            
            // メイン解析メソッド
            Namespace parse(int argc, char* argv[], const std::vector<std::shared_ptr<Argument>>& arguments) {
                std::vector<std::string> args;
                for (int i = 1; i < argc; ++i) {  // Skip program name
                    args.push_back(std::string(argv[i]));
                }
                return parse(args, arguments);
            }
            
            // string配列版の解析メソッド
            Namespace parse(const std::vector<std::string>& args, 
                          const std::vector<std::shared_ptr<Argument>>& arguments) {
                
                // 引数をタイプ別に分類
                _classify_arguments(arguments);
                
                // トークン化
                tokenizer_.tokenize(args);
                
                // 結果を保持するNamespace
                Namespace result;
                
                // デフォルト値を設定
                _set_default_values(result, arguments);
                
                // 位置引数のインデックス
                size_t positional_index = 0;
                
                // トークンを順次処理
                while (tokenizer_.has_next()) {
                    Token token = tokenizer_.next();
                    
                    switch (token.type) {
                        case Token::POSITIONAL:
                            _handle_positional_argument(token, result, positional_index);
                            break;
                            
                        case Token::SHORT_OPTION:
                        case Token::LONG_OPTION:
                            _handle_option_argument(token, result);
                            break;
                            
                        case Token::END_OPTIONS:
                            // "--" 以降はすべて位置引数として処理
                            while (tokenizer_.has_next()) {
                                Token pos_token = tokenizer_.next();
                                if (pos_token.type == Token::POSITIONAL) {
                                    _handle_positional_argument(pos_token, result, positional_index);
                                }
                            }
                            break;
                            
                        case Token::OPTION_VALUE:
                            // このケースは通常 _handle_option_argument で処理される
                            break;
                    }
                }
                
                // 必須引数のチェック
                _validate_required_arguments(result, arguments);
                
                return result;
            }
            
        private:
            // 引数を位置引数とオプション引数に分類
            void _classify_arguments(const std::vector<std::shared_ptr<Argument>>& arguments) {
                positional_args_.clear();
                option_args_.clear();
                
                for (const auto& arg : arguments) {
                    if (arg->is_positional()) {
                        positional_args_.push_back(arg);
                    } else {
                        // オプション引数は全ての名前でマップに登録
                        for (const auto& name : arg->get_names()) {
                            option_args_[name] = arg;
                        }
                    }
                }
            }
            
            // デフォルト値を設定
            void _set_default_values(Namespace& result, 
                                   const std::vector<std::shared_ptr<Argument>>& arguments) {
                for (const auto& arg : arguments) {
                    const auto& def = arg->definition();
                    if (!def.default_value.empty()) {
                        std::string key = _get_storage_key(arg);
                        result.set_raw(key, def.default_value);
                    }
                }
            }
            
            // 位置引数の処理（nargs対応）
            void _handle_positional_argument(const Token& token, Namespace& result, 
                                           size_t& positional_index) {
                if (positional_index >= positional_args_.size()) {
                    throw std::runtime_error("Too many positional arguments");
                }
                
                auto arg = positional_args_[positional_index];
                const auto& def = arg->definition();
                std::string key = _get_storage_key(arg);
                
                try {
                    std::vector<std::string> values;
                    values.push_back(token.value);  // 現在のトークンを追加
                    
                    // nargs処理による追加値の収集
                    if (def.nargs == -5) {  // remainder - 残り全て
                        while (tokenizer_.has_next()) {
                            values.push_back(tokenizer_.next().value);
                        }
                    } else if (def.nargs == -3) {  // "*" - zero or more (すでに1つあるので、残りを収集)
                        while (tokenizer_.has_next()) {
                            Token next = tokenizer_.peek();
                            if (next.type == Token::POSITIONAL) {
                                values.push_back(tokenizer_.next().value);
                            } else {
                                break;
                            }
                        }
                    } else if (def.nargs == -4) {  // "+" - one or more (すでに1つあるので、残りを収集)
                        while (tokenizer_.has_next()) {
                            Token next = tokenizer_.peek();
                            if (next.type == Token::POSITIONAL) {
                                values.push_back(tokenizer_.next().value);
                            } else {
                                break;
                            }
                        }
                    } else if (def.nargs > 1) {  // 固定数（2以上）
                        for (int i = 1; i < def.nargs; ++i) {
                            if (!tokenizer_.has_next()) {
                                throw std::runtime_error("Positional argument '" + key + "' requires " + std::to_string(def.nargs) + " values");
                            }
                            Token next = tokenizer_.next();
                            if (next.type != Token::POSITIONAL) {
                                throw std::runtime_error("Positional argument '" + key + "' requires " + std::to_string(def.nargs) + " values");
                            }
                            values.push_back(next.value);
                        }
                    }
                    
                    // 値の変換と格納
                    if (values.size() == 1 && def.nargs != -3 && def.nargs != -4 && def.nargs != -5 && def.nargs <= 1) {
                        // 単一値
                        AnyValue value = arg->convert_value(values[0]);
                        if (!arg->validate_value(value)) {
                            throw std::invalid_argument("Invalid value for argument '" + key + "': " + values[0]);
                        }
                        result.set_raw(key, value);
                    } else {
                        // 複数値、文字列リストとして格納
                        result.set(key, values);
                    }
                    
                    ++positional_index;
                } catch (const std::exception& e) {
                    throw std::invalid_argument("Error parsing positional argument '" + key + "': " + e.what());
                }
            }
            
            // オプション引数の処理
            void _handle_option_argument(const Token& token, Namespace& result) {
                auto it = option_args_.find(token.value);
                if (it == option_args_.end()) {
                    throw std::runtime_error("Unknown argument: " + token.value);
                }
                
                auto arg = it->second;
                const auto& def = arg->definition();
                std::string key = _get_storage_key(arg);
                
                // actionに基づく処理
                if (def.action == "help") {
                    // ヘルプが要求された場合は例外を投げる
                    throw help_requested("Help requested");
                } else if (def.action == "store_true") {
                    result.set(key, true);
                } else if (def.action == "store_false") {
                    result.set(key, false);
                } else if (def.action == "count") {
                    // カウント数を増加
                    int current_count = 0;
                    if (result.has(key)) {
                        current_count = result.get<int>(key);
                    }
                    result.set(key, current_count + 1);
                } else if (def.action == "append") {
                    // リストに値を追加
                    if (!tokenizer_.has_next()) {
                        throw std::runtime_error("Argument " + token.value + " requires a value");
                    }
                    
                    Token value_token = tokenizer_.next();
                    if (value_token.type != Token::OPTION_VALUE && value_token.type != Token::POSITIONAL) {
                        throw std::runtime_error("Argument " + token.value + " requires a value");
                    }
                    
                    try {
                        AnyValue value = arg->convert_value(value_token.value);
                        if (!arg->validate_value(value)) {
                            throw std::invalid_argument("Invalid value for argument " + token.value + ": " + value_token.value);
                        }
                        
                        // 既存のリストに追加、またはリストを作成
                        if (result.has(key)) {
                            const auto& current_list = result.get_raw(key);
                            // ListハンドラーでリストAPI処理が必要だが、簡単のため文字列ベクターで実装
                            if (current_list.type() == typeid(std::vector<std::string>)) {
                                auto list = current_list.get<std::vector<std::string>>();
                                list.push_back(value.get<std::string>());
                                result.set(key, list);
                            } else {
                                // 初回、単一値を含むリストに変換
                                std::vector<std::string> new_list;
                                new_list.push_back(current_list.get<std::string>());
                                new_list.push_back(value.get<std::string>());
                                result.set(key, new_list);
                            }
                        } else {
                            // 初回はリストとして作成
                            std::vector<std::string> new_list;
                            new_list.push_back(value.get<std::string>());
                            result.set(key, new_list);
                        }
                    } catch (const std::invalid_argument& e) {
                        throw;  // Re-throw invalid_argument as-is for type conversion errors
                    } catch (const std::exception& e) {
                        throw std::runtime_error("Error parsing argument " + token.value + ": " + e.what());
                    }
                } else if (def.action == "custom") {
                    // カスタムアクション処理
                    if (!def.custom_action) {
                        throw std::runtime_error("Custom action specified but no handler provided for " + token.value);
                    }
                    
                    // 値が必要かどうかはnargによる（この場合は簡単のため値を取る）
                    std::string value_str = "";
                    if (tokenizer_.has_next()) {
                        Token value_token = tokenizer_.peek();
                        if (value_token.type == Token::OPTION_VALUE || value_token.type == Token::POSITIONAL) {
                            value_token = tokenizer_.next();
                            value_str = value_token.value;
                        }
                    }
                    
                    try {
                        AnyValue current_value;
                        if (result.has(key)) {
                            current_value = result.get_raw(key);
                        }
                        
                        AnyValue new_value = def.custom_action(current_value, value_str);
                        result.set_raw(key, new_value);
                    } catch (const std::exception& e) {
                        throw std::invalid_argument("Error in custom action for " + token.value + ": " + e.what());
                    }
                } else if (def.action == "store" || def.action.empty()) {
                    // nargs処理による値の収集
                    try {
                        std::vector<std::string> values = _collect_values(def, token);
                        
                        if (values.empty()) {
                            // nargs="?" で値が無い場合、デフォルト値を使用
                            if (def.nargs == -2 && !def.default_value.empty()) {
                                result.set_raw(key, def.default_value);
                            } else if (def.nargs == -2) {
                                // デフォルト値もない場合はNone的な扱い（設定しない）
                            } else if (def.nargs == -3) {
                                // nargs="*" の場合、空のリストを設定
                                result.set(key, std::vector<std::string>());
                            } else {
                                throw std::runtime_error("Argument " + token.value + " requires a value");
                            }
                        } else if (values.size() == 1 && def.nargs != -3 && def.nargs != -4 && def.nargs <= 1) {
                            // 単一値の場合
                            AnyValue value = arg->convert_value(values[0]);
                            if (!arg->validate_value(value)) {
                                throw std::invalid_argument("Invalid value for argument " + token.value + ": " + values[0]);
                            }
                            result.set_raw(key, value);
                        } else {
                            // 複数値の場合、元の文字列のリストとして格納
                            result.set(key, values);
                        }
                    } catch (const std::invalid_argument& e) {
                        throw;  // Re-throw invalid_argument as-is for type conversion errors
                    } catch (const std::exception& e) {
                        throw std::runtime_error("Error parsing argument " + token.value + ": " + e.what());
                    }
                } else {
                    throw std::runtime_error("Unsupported action: " + def.action);
                }
            }
            
            // 必須引数のチェック
            void _validate_required_arguments(const Namespace& result, 
                                            const std::vector<std::shared_ptr<Argument>>& arguments) {
                for (const auto& arg : arguments) {
                    if (arg->definition().required) {
                        std::string key = _get_storage_key(arg);
                        if (!result.has(key)) {
                            throw std::runtime_error("Required argument missing: " + arg->get_name());
                        }
                    }
                }
            }
            
            // nargs値を取得（固定数、特殊値の処理）
            int _get_effective_nargs(const ArgumentDefinition& def) const {
                switch (def.nargs) {
                    case -2: return 0;  // "?" - optional (0 or 1)
                    case -3: return 0;  // "*" - zero or more
                    case -4: return 1;  // "+" - one or more
                    case -5: return 0;  // "remainder" - all remaining
                    default: return def.nargs;  // 固定数
                }
            }
            
            // nargs仕様に基づいて値を収集
            std::vector<std::string> _collect_values(const ArgumentDefinition& def, const Token& initial_token) {
                std::vector<std::string> values;
                
                if (def.nargs == -2) {  // "?" - 0 or 1
                    if (tokenizer_.has_next()) {
                        Token next = tokenizer_.peek();
                        if (next.type == Token::OPTION_VALUE || next.type == Token::POSITIONAL) {
                            values.push_back(tokenizer_.next().value);
                        }
                    }
                } else if (def.nargs == -3) {  // "*" - 0 or more
                    while (tokenizer_.has_next()) {
                        Token next = tokenizer_.peek();
                        if (next.type == Token::OPTION_VALUE || 
                            (next.type == Token::POSITIONAL && next.value[0] != '-')) {
                            values.push_back(tokenizer_.next().value);
                        } else {
                            break;
                        }
                    }
                } else if (def.nargs == -4) {  // "+" - 1 or more
                    if (!tokenizer_.has_next()) {
                        throw std::runtime_error("Argument requires at least one value");
                    }
                    
                    // 最初の値を取得
                    Token first = tokenizer_.next();
                    if (first.type != Token::OPTION_VALUE && first.type != Token::POSITIONAL) {
                        throw std::runtime_error("Argument requires at least one value");
                    }
                    values.push_back(first.value);
                    
                    // 追加の値を収集
                    while (tokenizer_.has_next()) {
                        Token next = tokenizer_.peek();
                        if (next.type == Token::OPTION_VALUE || 
                            (next.type == Token::POSITIONAL && next.value[0] != '-')) {
                            values.push_back(tokenizer_.next().value);
                        } else {
                            break;
                        }
                    }
                } else if (def.nargs == -5) {  // "remainder" - all remaining
                    // 残りの全てのトークンを収集
                    while (tokenizer_.has_next()) {
                        values.push_back(tokenizer_.next().value);
                    }
                } else if (def.nargs > 0) {  // 固定数
                    for (int i = 0; i < def.nargs; ++i) {
                        if (!tokenizer_.has_next()) {
                            throw std::runtime_error("Argument requires " + std::to_string(def.nargs) + " values");
                        }
                        
                        Token value_token = tokenizer_.next();
                        if (value_token.type != Token::OPTION_VALUE && value_token.type != Token::POSITIONAL) {
                            throw std::runtime_error("Argument requires " + std::to_string(def.nargs) + " values");
                        }
                        values.push_back(value_token.value);
                    }
                } else if (def.nargs == 0) {  // 値を取らない
                    // store_true, store_false, count等の処理
                } else {  // デフォルト（1つの値）
                    if (!tokenizer_.has_next()) {
                        throw std::runtime_error("Argument requires a value");
                    }
                    
                    Token value_token = tokenizer_.next();
                    if (value_token.type != Token::OPTION_VALUE && value_token.type != Token::POSITIONAL) {
                        throw std::runtime_error("Argument requires a value");
                    }
                    values.push_back(value_token.value);
                }
                
                return values;
            }
            
            // 引数の保存キーを取得（位置引数は名前、オプション引数は主要名前）
            std::string _get_storage_key(const std::shared_ptr<Argument>& arg) const {
                const auto& names = arg->get_names();
                if (names.empty()) {
                    return "unnamed";
                }
                
                if (arg->is_positional()) {
                    return names[0];
                }
                
                // オプション引数の場合、長形式を優先
                for (const auto& name : names) {
                    if (name.length() > 2 && name.substr(0, 2) == "--") {
                        return name.substr(2);  // "--" を除去
                    }
                }
                
                // 長形式がない場合は短形式を使用
                std::string short_name = names[0];
                if (short_name.length() >= 2 && short_name[0] == '-') {
                    return short_name.substr(1);  // "-" を除去
                }
                
                return names[0];
            }
        };
    } // namespace detail
    
    // HelpGenerator implementation (after ArgumentParser definition)
    inline std::string detail::HelpGenerator::generate_help(const ArgumentParser& parser) {
        std::ostringstream oss;
        
        // Usage line
        oss << "usage: " << parser.prog();
        
        // Build usage line with arguments
        const auto& arguments = parser.get_arguments();
        std::vector<std::shared_ptr<Argument>> positionals;
        std::vector<std::shared_ptr<Argument>> optionals;
        
        // 引数を分類
        for (const auto& arg : arguments) {
            if (arg->is_positional()) {
                positionals.push_back(arg);
            } else {
                optionals.push_back(arg);
            }
        }
        
        // オプション引数を使用法に追加（ヘルプオプションは除外）
        for (const auto& arg : optionals) {
            const auto& def = arg->definition();
            const auto& names = arg->get_names();
            if (names.empty()) continue;
            
            // ヘルプオプション（--help, -h）は使用法には表示しない
            bool is_help_option = false;
            for (const auto& name : names) {
                if (name == "--help" || name == "-h") {
                    is_help_option = true;
                    break;
                }
            }
            if (is_help_option) continue;
            
            // 必須オプションかどうか
            if (def.required) {
                oss << " ";
            } else {
                oss << " [";
            }
            
            // 最初の名前を使用（通常は短形式）
            oss << names[0];
            
            // metavarまたはnargsに基づいた値表示
            if (def.action != "store_true" && def.action != "store_false" && def.action != "count") {
                std::string metavar = def.metavar;
                if (metavar.empty()) {
                    // デフォルトのmetavar生成
                    std::string base = names[0];
                    if (base.length() > 2 && base.substr(0, 2) == "--") {
                        metavar = base.substr(2);
                    } else if (base.length() > 1 && base[0] == '-') {
                        metavar = base.substr(1);
                    } else {
                        metavar = base;
                    }
                    // 大文字に変換
                    for (char& c : metavar) {
                        c = std::toupper(c);
                    }
                }
                
                // nargsに基づいた表示
                if (def.nargs == -2) {  // "?"
                    oss << " [" << metavar << "]";
                } else if (def.nargs == -3) {  // "*"
                    oss << " [" << metavar << " [" << metavar << " ...]]";
                } else if (def.nargs == -4) {  // "+"
                    oss << " " << metavar << " [" << metavar << " ...]";
                } else if (def.nargs > 1) {
                    for (int i = 0; i < def.nargs; ++i) {
                        oss << " " << metavar;
                    }
                } else {
                    oss << " " << metavar;
                }
            }
            
            if (!def.required) {
                oss << "]";
            }
        }
        
        // 位置引数を使用法に追加
        for (const auto& arg : positionals) {
            const auto& def = arg->definition();
            std::string name = arg->get_name();
            std::string metavar = def.metavar.empty() ? name : def.metavar;
            
            oss << " ";
            
            // nargsに基づいた表示
            if (def.nargs == -2) {  // "?"
                oss << "[" << metavar << "]";
            } else if (def.nargs == -3) {  // "*"
                oss << "[" << metavar << " [" << metavar << " ...]]";
            } else if (def.nargs == -4) {  // "+"
                oss << metavar << " [" << metavar << " ...]";
            } else if (def.nargs == -5) {  // remainder
                oss << metavar << " ...";
            } else if (def.nargs > 1) {
                for (int i = 0; i < def.nargs; ++i) {
                    if (i > 0) oss << " ";
                    oss << metavar;
                }
            } else {
                if (!def.required) {
                    oss << "[" << metavar << "]";
                } else {
                    oss << metavar;
                }
            }
        }
        
        oss << "\n";
        
        // Description
        if (!parser.description().empty()) {
            oss << "\n" << parser.description() << "\n";
        }
        
        // Positional arguments section
        if (!positionals.empty()) {
            oss << "\npositional arguments:\n";
            for (const auto& arg : positionals) {
                const auto& def = arg->definition();
                std::string name = arg->get_name();
                std::string metavar = def.metavar.empty() ? name : def.metavar;
                
                // フォーマット: 名前を左寄せ、説明を右側に
                oss << "  ";
                std::string arg_str = metavar;
                oss << std::left << std::setw(20) << arg_str;
                
                if (!def.help.empty()) {
                    // ヘルプテキストを折り返し（80文字幅を考慮）
                    std::string help_text = def.help;
                    size_t max_width = 80 - 24;  // 80文字 - インデント - 名前の幅
                    
                    if (help_text.length() <= max_width) {
                        oss << help_text;
                    } else {
                        // 長いヘルプテキストの折り返し処理
                        size_t pos = 0;
                        bool first_line = true;
                        while (pos < help_text.length()) {
                            if (!first_line) {
                                oss << "\n" << std::string(24, ' ');  // 24文字のインデント
                            }
                            size_t line_length = std::min(max_width, help_text.length() - pos);
                            oss << help_text.substr(pos, line_length);
                            pos += line_length;
                            first_line = false;
                        }
                    }
                }
                oss << "\n";
            }
        }
        
        // Optional arguments section
        if (!optionals.empty() || true) {  // 常にオプション引数セクションを表示
            oss << "\noptional arguments:\n";
            
            // 全てのオプション引数を表示（ヘルプ引数も含む）
            for (const auto& arg : optionals) {
                const auto& def = arg->definition();
                const auto& names = arg->get_names();
                if (names.empty()) continue;
                
                oss << "  ";
                
                // 引数名を短形式優先で並び替え
                std::vector<std::string> sorted_names = names;
                std::sort(sorted_names.begin(), sorted_names.end(), [](const std::string& a, const std::string& b) {
                    // 短形式（-x）を長形式（--xxx）より先に
                    if (a.length() != b.length()) {
                        return a.length() < b.length();
                    }
                    return a < b;
                });
                
                // 引数名とmetavarの組み立て
                std::string arg_str;
                for (size_t i = 0; i < sorted_names.size(); ++i) {
                    if (i > 0) arg_str += ", ";
                    arg_str += sorted_names[i];
                }
                
                // metavarを追加（action次第）
                if (def.action != "store_true" && def.action != "store_false" && def.action != "count" && def.action != "help") {
                    std::string metavar = def.metavar;
                    if (metavar.empty()) {
                        // デフォルトのmetavar生成
                        for (const auto& name : sorted_names) {
                            if (name.length() > 2 && name.substr(0, 2) == "--") {
                                metavar = name.substr(2);
                                break;
                            }
                        }
                        if (metavar.empty() && !sorted_names.empty()) {
                            metavar = sorted_names[0];
                            if (metavar.length() > 1 && metavar[0] == '-') {
                                metavar = metavar.substr(1);
                            }
                        }
                        // 大文字に変換
                        for (char& c : metavar) {
                            c = std::toupper(c);
                        }
                    }
                    
                    if (!metavar.empty()) {
                        arg_str += " " + metavar;
                    }
                }
                
                oss << std::left << std::setw(20) << arg_str;
                
                // ヘルプテキスト
                if (!def.help.empty()) {
                    std::string help_text = def.help;
                    
                    // デフォルト値を表示
                    if (!def.default_value.empty() && def.action == "store") {
                        help_text += " (default: ";
                        if (def.default_value.type() == typeid(std::string)) {
                            help_text += def.default_value.get<std::string>();
                        } else if (def.default_value.type() == typeid(int)) {
                            help_text += std::to_string(def.default_value.get<int>());
                        } else if (def.default_value.type() == typeid(double)) {
                            help_text += std::to_string(def.default_value.get<double>());
                        } else if (def.default_value.type() == typeid(bool)) {
                            help_text += def.default_value.get<bool>() ? "true" : "false";
                        }
                        help_text += ")";
                    }
                    
                    // 折り返し処理
                    size_t max_width = 80 - 24;
                    if (help_text.length() <= max_width) {
                        oss << help_text;
                    } else {
                        size_t pos = 0;
                        bool first_line = true;
                        while (pos < help_text.length()) {
                            if (!first_line) {
                                oss << "\n" << std::string(24, ' ');
                            }
                            size_t line_length = std::min(max_width, help_text.length() - pos);
                            oss << help_text.substr(pos, line_length);
                            pos += line_length;
                            first_line = false;
                        }
                    }
                }
                oss << "\n";
            }
        }
        
        // Epilog
        if (!parser.epilog().empty()) {
            oss << "\n" << parser.epilog() << "\n";
        }
        
        return oss.str();
    }
    
    // ArgumentParser parse_args method implementations
    inline Namespace ArgumentParser::parse_args(int argc, char* argv[]) {
        // Set program name from argv[0] if not already set (default or empty)
        if ((prog_ == "program" || prog_.empty()) && argc > 0) {
            prog_ = _extract_prog_name(std::string(argv[0]));
        }
        
        try {
            detail::Parser parser;
            return parser.parse(argc, argv, arguments_);
        } catch (const help_requested&) {
            // Generate and throw help message
            std::string help_message = detail::HelpGenerator::generate_help(*this);
            throw help_requested(help_message);
        }
    }
    
    inline Namespace ArgumentParser::parse_args(const std::vector<std::string>& args) {
        try {
            detail::Parser parser;
            return parser.parse(args, arguments_);
        } catch (const help_requested&) {
            // Generate and throw help message
            std::string help_message = detail::HelpGenerator::generate_help(*this);
            throw help_requested(help_message);
        }
    }
    
} // namespace argparse

#endif // ARGPARSE_HPP_INCLUDED