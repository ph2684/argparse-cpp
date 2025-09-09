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
    
} // namespace argparse

#endif // ARGPARSE_HPP_INCLUDED