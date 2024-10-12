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

    JsonObject asObject(){
        return getValue<JsonObject>();
    }


    template<typename T>
    T getValue(){
        if(std::holds_alternative<T>(value)){
            return std::get<T>(value);
        }
        else{
            throw std::bad_variant_access();
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

    static JsonValue Object(const std::initializer_list<std::pair<const char*, JsonValue>>& list){
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

    void write(const std::string& fileName){
        std::ofstream file(fileName);
        file << *this;
        file.close();
    }

private:
    std::unordered_map<std::string, JsonValue> data;
};

// TODO //
// - Syntax checking
// - Add support for comments?

class JsonParse{
public:

    JsonParse() = default;
    ~JsonParse() = default;

    Json parse(const std::string& fileName){
        Json json;

        std::ifstream file{fileName};
        std::stringstream buffer;
        buffer << file.rdbuf();
        Tokenizer tokenizer(buffer.str());
        tokenizer.tokenize();
        for(auto token = tokenizer.tokenize(); token.type != TokenType::END; token = tokenizer.tokenize()){
            switch(token.type){
                case TokenType::COMMA:
                    break;
                case TokenType::STRING:
                    key = token.value;
                    break;
                case TokenType::LEFT_BRACE:
                    json[key] = parseObject(tokenizer);
                    break;
                case TokenType::LEFT_BRACKET:
                    json[key] = parseArray(tokenizer);
                    break;
                default:
                    break;

            }
        }
        return json;
    }

private:
    std::string key{};

    JsonValue parseObject(Tokenizer& tokenizer){
        JsonObject map;
        std::pair<std::string, std::shared_ptr<JsonValue>> pair;
        bool keyFound = false;
        for(Token token = tokenizer.tokenize(); token.type != TokenType::RIGHT_BRACE; token = tokenizer.tokenize()){
            if(!keyFound){
                pair.first = token.value;
                keyFound = true;
            }
            else{
                switch(token.type){
                    case TokenType::COLON:
                        break;
                    case TokenType::COMMA:
                        keyFound = false;
                        break;
                    case TokenType::LEFT_BRACE:
                        pair.second = std::make_shared<JsonValue>(parseObject(tokenizer));
                        break;
                    case TokenType::LEFT_BRACKET:
                        pair.second = std::make_shared<JsonValue>(parseArray(tokenizer));
                        break;
                    case TokenType::INTEGER:
                        pair.second = std::make_shared<JsonValue>(std::stoi(token.value));
                        break;
                    case TokenType::FLOAT:
                        pair.second = std::make_shared<JsonValue>(std::stod(token.value));
                        break;
                    case TokenType::TRUE:
                        pair.second = std::make_shared<JsonValue>(true);
                        break;
                    case TokenType::FALSE:
                        pair.second = std::make_shared<JsonValue>(false);
                        break;
                    case TokenType::NULL_TYPE:
                        pair.second = std::make_shared<JsonValue>(Json::Null());
                        break;
                    default:
                        pair.second = std::make_shared<JsonValue>(token.value);
                        break;
                }
            }
            map[pair.first] = pair.second;
        }
        return map;
    }

    JsonValue parseArray(Tokenizer& tokenizer){
        JsonArray array;
        for(Token token = tokenizer.tokenize(); token.type != TokenType::RIGHT_BRACKET; token = tokenizer.tokenize()){
            switch(token.type){
                case TokenType::COMMA:
                        break;
                    case TokenType::LEFT_BRACE:
                        array.push_back(std::make_shared<JsonValue>(parseObject(tokenizer)));
                        break;
                    case TokenType::LEFT_BRACKET:
                        array.push_back(std::make_shared<JsonValue>(parseArray(tokenizer)));
                        break;
                    case TokenType::INTEGER:
                        array.push_back(std::make_shared<JsonValue>(std::stoi(token.value)));
                        break;
                    case TokenType::FLOAT:
                        array.push_back(std::make_shared<JsonValue>(std::stod(token.value)));
                        break;
                    case TokenType::TRUE:
                        array.push_back(std::make_shared<JsonValue>(true));
                        break;
                    case TokenType::FALSE:
                        array.push_back(std::make_shared<JsonValue>(false));
                        break;
                    case TokenType::NULL_TYPE:
                        array.push_back(std::make_shared<JsonValue>(Json::Null()));
                        break;
                    default:
                        array.push_back(std::make_shared<JsonValue>(token.value));
                        break;
            }

        } 
        return array;
    }

};

#endif