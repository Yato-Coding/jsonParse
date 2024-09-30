#ifndef JSONPARSE_HPP
#define JSONPARSE_HPP

#include "tokenizer.hpp"

#include <initializer_list>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <ostream>
#include <memory>
#include <fstream>
#include <iostream>
#include <sstream>

class JsonValue;

using JsonArray = std::vector<std::shared_ptr<JsonValue>>;
using JsonObject = std::unordered_map<std::string, std::shared_ptr<JsonValue>>;


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
    JsonValue(JsonObject& val) : value(val){}

    ~JsonValue() = default;

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
            else{
                os << arg;
            }
        }, json.value);
        return os;
    }

private:
    std::variant<std::monostate, int, bool, double, std::string, char, JsonArray, JsonObject> value;
};

class Json{
public:
    Json() = default;
    ~Json() = default;

    JsonValue& operator[](const std::string& key) {
        return data[key];
    }

    static JsonValue Array(const std::initializer_list<JsonValue>& list){
        JsonArray arr;
        for(const auto& value : list){
            arr.push_back(std::make_shared<JsonValue>(value));
        }
        return JsonValue(arr);
    }

    static JsonValue Object(const std::initializer_list<std::pair<std::string, JsonValue>>& list){
        JsonObject map;
        for(const auto& [key, val] : list){
            map[key] = std::make_shared<JsonValue>(val);
        }
        return JsonValue(map);
    }

    static JsonValue Null(){
        return JsonValue();
    }

    friend std::ostream& operator<<(std::ostream& os, const Json& json){
        os << '{';
        bool first = true;
        for(const auto& [key, value] : json.data){
            if(!first){
                os << ',';
            }
            first = false;
            os << '"' << key << '"' << ':' << value;
        }
        return os << '}';
    }

    // TODO //
    // - Think about splitting the parsing into its own class.
    //   It does get rather complex later on, especially if you want to support syntax checking.
    //   I'll leave it as is for now, for debugging purposes.
    //
    // - Syntax checking
    // - Add support for comments?
    // - Actually parse the input

    friend std::ifstream& operator>>(std::ifstream& is, const Json& json){
        std::stringstream buffer;
        buffer << is.rdbuf();
        Tokenizer tokenizer(buffer.str());
        for(auto token = tokenizer.tokenize(); token.type != TokenType::END; token = tokenizer.tokenize()){
            std::cout << token.value << std::endl;
        }
        return is;
    }

    void write(const std::string& fileName){
        std::ofstream file(fileName);
        file << *this;
        file.close();
    }

    void parse(const std::string& fileName){
        std::ifstream file(fileName);
        file >> *this;
        file.close();
    }

private:
    std::unordered_map<std::string, JsonValue> data;
};

#endif