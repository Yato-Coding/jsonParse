#ifndef JSON_VALUE_HPP
#define JSON_VALUE_HPP

#include <memory>
#include <stdexcept>
#include "iterator.hpp"
#include <ostream>

class JsonValue{
public:
    JsonValue() : value(std::monostate{}){};
    JsonValue(int val) : value(val){}
    JsonValue(bool val) : value(val){}
    JsonValue(double val) : value(val){}
    JsonValue(const std::string& val) : value(val){}
    JsonValue(const char* val) : value(std::string(val)){} // C Style Strings, like when passing "String" directly
    JsonValue(char val) : value(std::string(1, val)){}
    JsonValue(JsonArray& val) : value(val){}
    JsonValue(JsonObject val) : value(val){}

    ~JsonValue() = default;

    template<typename T>
    auto begin(){
        if(auto ptr = std::get_if<T>(&value)){
            return ptr->begin();
        }
        throw std::runtime_error("Not an object or array");
    }

    template<typename T>
    auto end(){
        if(auto ptr = std::get_if<T>(&value)){
            return ptr->end();
        }
        throw std::runtime_error("Not an object or array");
    }

    Iterator begin() {
        if(isArray()){
            return Iterator(begin<JsonArray>());
        }else if(isObject()){
            return Iterator(begin<JsonObject>());
        }
        throw std::runtime_error("JsonValue is neither an array nor an object");
    }

    Iterator end() {
        if (isArray()) {
            return Iterator(end<JsonArray>());
        } else if (isObject()) {
            return Iterator(end<JsonObject>());
        }
        throw std::runtime_error("JsonValue is neither an array nor an object");
    }

    bool isNull() const { return std::holds_alternative<std::monostate>(value); }
    bool isInt() const { return std::holds_alternative<int>(value); }
    bool isBool() const { return std::holds_alternative<bool>(value);; }
    bool isDouble() const { return std::holds_alternative<double>(value);; }
    bool isString() const { return std::holds_alternative<std::string>(value);; }
    bool isArray() const { return std::holds_alternative<JsonArray>(value); }
    bool isObject() const { return std::holds_alternative<JsonObject>(value); } 

    template<typename T>
    operator T(){
        return getValue<T>();
    }

    JsonValue& operator[](const std::string& key){
        if(auto obj = std::get_if<JsonObject>(&value)){
            auto it = obj->find(key);
            if(it != obj->end()){
                return *it->second;
            }
            throw std::runtime_error("The Key doesnt exist");
        }
        else{
            throw std::runtime_error(key + " is not an object");
        }
    }

    // just for string literals
    JsonValue& operator[](const char* key){
        return operator[](std::string(key));
    }

    JsonValue& operator[](long unsigned int index){
        if(auto arr = std::get_if<JsonArray>(&value)){
            return *arr->at(index);
        }
        else{
            throw std::runtime_error("Not an array");
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const JsonValue& json){
        std::visit([&os](auto&& arg){
            using T = std::decay_t<decltype(arg)>;
            if constexpr(std::is_same_v<T, std::string>){
                os << '"' << arg << '"';
            }
            else if constexpr(std::is_same_v<T, JsonArray>){
                os << '[';
                bool first = true;
                for(const auto& val : arg){
                    if(!first){
                        os << ',';
                    }
                    first = false;
                    os << *val;
                }
                os << ']';
            }
            else if constexpr(std::is_same_v<T, JsonObject>){
                os << '{';
                bool first = true;
                for(const auto& [key, val] : arg){
                    if(!first){
                        os << ',';
                    }
                    first = false;
                    os << '"' << key << '"' << ':' << *val;
                }
                os << '}';
            }
            else if constexpr(std::is_same_v<T, std::monostate>){
                os << "null";
            }
            else if constexpr(std::is_same_v<T, bool>){
                // bool broke, dont ask why, this fixes it 
                if(arg == 0){
                    os << "false";
                }
                else{
                    os << "true";
                }
            }
            else{
                os << arg;
            }
        }, json.value);
        return os;
    }

private:
    std::variant<std::monostate, int, bool, double, std::string, char, JsonArray, JsonObject> value;

    template<typename T>
    T getValue(){
        if(std::holds_alternative<T>(value)){
            return std::get<T>(value);
        }
        else{
            throw std::bad_variant_access();
        }
    }    

    
};

#endif