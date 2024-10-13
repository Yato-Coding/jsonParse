// TODO
// split this into their own functions, dear god, this shit is headache inducing 

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
class JsonParse;

using JsonArray = std::vector<std::shared_ptr<JsonValue>>;
using JsonObject = std::unordered_map<std::string, std::shared_ptr<JsonValue>>;


// TODO
// Fix the operator* class, JsonObject completely shits the bed 
// because it yields a std::pair<std::string, std::shared_ptr>>
// might actually need a JsonArray/Object wrapper with their own iterators or smth?
// need to be able to return both std::pair<> and std::shared_ptr<JsonValue> in begin()/end()
// Can just create a JsonObject variable and use that in the foreach to use the stdlib iterator 
// instead of this piece of garbage code as hotfix. 

class Iterator {
    public:
        using variant_iterator = std::variant<JsonArray::iterator, JsonObject::iterator>;

        Iterator(variant_iterator variant_it) : it(variant_it) {}

        // Prefix increment
        Iterator& operator++() {
            std::visit([](auto& iter) { ++iter; }, it);
            return *this;
        }

        // Dereference operator
        std::shared_ptr<JsonValue> operator*() {
            return std::visit([](auto& iter) -> std::shared_ptr<JsonValue> {
                if constexpr (std::is_same_v<std::decay_t<decltype(iter)>, JsonArray::iterator>) {
                    return *iter;
                } else if constexpr (std::is_same_v<std::decay_t<decltype(iter)>, JsonObject::iterator>) {
                    JsonObject obj;
                    obj[iter->first] = iter->second;
                    return std::make_shared<JsonValue>(std::move(obj)); // Wrap in a shared_ptr<JsonValue>
                }
                throw std::runtime_error("Unsupported iterator type");
            }, it);
        } 

        bool operator!=(const Iterator& other) const {
            return it != other.it;
        }

    private:
        variant_iterator it;
    };

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