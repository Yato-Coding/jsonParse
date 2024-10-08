#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <string>
#include <cmath>
#include <stdexcept>

enum struct TokenType{
    LEFT_BRACE,  // {
    RIGHT_BRACE, // }
    LEFT_BRACKET, // [
    RIGHT_BRACKET, // ]
    COMMA,       // ,
    COLON,       // :
    STRING,      // "..."
    NUMBER,      // 123, -12.34
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
                throw std::invalid_argument("Invalid character: " + std::string(1, c));
            }
    }

private:
    std::string input;
    long unsigned int pos;

    Token parseString(){
        pos++; // skip starting quote
        std::string value{};
        while(input[pos] != '"' && pos < input.size()){
            value += input[pos];
            pos++;
        }
        pos++ ; // skip ending quote
        return {TokenType::STRING, value};
    }

    Token parseDigit(){
        std::string value{};
        while((std::isdigit(input[pos]) || input[pos] == '.' || input[pos] == '-') && (pos < input.size())){
            value += input[pos];
            pos++;
        }
        return {TokenType::NUMBER, value};
    }

    void skipWhitespace(){
        while(std::isspace(input[pos]) && pos < input.size()) pos++;
    }
};

#endif