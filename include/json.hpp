#ifndef JSON 
#define JSON 

#include "tokenizer.hpp"
#include "jsonValue.hpp"

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <fstream>
#include <iostream>
#include <sstream>

class JsonValue;
class JsonParse;

using JsonArray = std::vector<std::shared_ptr<JsonValue>>;
using JsonObject = std::unordered_map<std::string, std::shared_ptr<JsonValue>>;

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


// TODO
// - Syntax checking?
// - Add support for comments?
// - [] and {} support at the start of the file. 
// Maybe have to move away from the idea that everythings always a key value pair?

// TODO
// - Split JsonParse and Json into different files
// - Make JsonParse callable from within Json

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
                        pair.second = std::make_shared<JsonValue>();
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
                        array.push_back(std::make_shared<JsonValue>());
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
