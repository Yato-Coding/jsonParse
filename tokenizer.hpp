#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <string>
#include <cmath>
#include <stdexcept>
#include <iostream>

enum struct TokenType{
    LEFT_BRACE,  // {
    RIGHT_BRACE, // }
    LEFT_BRACKET, // [
    RIGHT_BRACKET, // ]
    COMMA,       // ,
    COLON,       // :
    STRING,      // "..."
    INTEGER,     // 123, -1029
    FLOAT,       // 12.34, -13.45
    TRUE,        // true
    FALSE,       // false
    NULL_TYPE,   // null
    END          // End of input
};

struct Token{
    TokenType type;
    std::string value;
};

class Tokenizer{
public:
    Tokenizer(const std::string& jsonInput) : input(jsonInput), pos(0){}
    ~Tokenizer() = default;

    Token tokenize(){
        skipWhitespace();
        if(pos > input.size()-1){ return {TokenType::END, ""}; }
        char c = input[pos];
        switch(c){
            case '{': pos++; return {TokenType::LEFT_BRACE, "{"};
            case '}': pos++; return {TokenType::RIGHT_BRACE, "}"};
            case '[': pos++; return {TokenType::LEFT_BRACKET, "["};
            case ']': pos++; return {TokenType::RIGHT_BRACKET, "]"};
            case ',': pos++; return {TokenType::COMMA, ","};
            case ':': pos++; return {TokenType::COLON, ":"};
            case '"': return parseString();
            default:
                if(std::isdigit(c) || c == '-'){ return parseDigit(); }
                if(c == 't'){ pos += 4; return {TokenType::TRUE, "true"}; };
                if(c == 'f'){ pos += 5; return {TokenType::FALSE, "false"}; };
                if(c == 'n'){ pos += 4;return {TokenType::NULL_TYPE, "null"}; };
                throw std::invalid_argument("Invalid character: " + input.substr(pos-5, 25) + " position: " + std::to_string(pos));
            }
    }

private:
    std::string input;
    long unsigned int pos;

    Token parseString(){
        pos++; // skip starting quote
        std::string value{};
        bool end_of_input = false;
        bool backslashed_string = false;

        while(!end_of_input){
            if(pos > input.size()){
                end_of_input = true;
                break;
            }


            if(input[pos] == '\\' && !backslashed_string){
                backslashed_string = true;
                value += input[pos];
            }
            else if(input[pos] == '\\' && backslashed_string){
                backslashed_string = false;
                value += input[pos];
            }
            else if(input[pos] == '"' && !backslashed_string){
               end_of_input = true;
               break; 
            }
            else{
                backslashed_string = false;
                value += input[pos];
            }

            pos++;
        }
        pos++ ; // skip ending quote
        return {TokenType::STRING, value};
    }

    Token parseDigit(){
        std::string value{};
        bool isFloat = false;
        while((std::isdigit(input[pos]) || input[pos] == '.' || input[pos] == '-') && (pos < input.size())){
            if(input[pos] == '.'){
                isFloat = true;
            }
            value += input[pos];
            pos++;
        }
        return {(!isFloat) ? TokenType::INTEGER : TokenType::FLOAT, value}; 
    }

    void skipWhitespace(){
        while(std::isspace(input[pos]) && pos < input.size()) pos++;
    }
};

#endif