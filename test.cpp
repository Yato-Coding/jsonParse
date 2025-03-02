#include "include/json.hpp"
#include <iostream>

int main(){
    // writing test
    Json json;

    json["person"] = Json::Object({
        {"name", "John Doe"},
        {"age", 3.124},
        {"hobbies", Json::Array({
            {"running"},
            {"singing"},
            {"dancing"}
        })},
        {"address", Json::Object({
            {"street", "123 Main St"},
            {"city", "Somewhere"},
            {"zipcode", 12345},
        })}});    

    // parsing test
    // JsonParse jsonParse;
    // Json parse = jsonParse.parse("large-file.json");
    // parse.write("testFile.json");

    // for loops
    // for(const auto& value : json["person"]["hobbies"]){
    //     // std::cout << value.first << ' ' << *value.second << '\n';
    //     std::cout << *value << '\n';
    // }
    //
    // JsonObject obj = json["person"]["address"];
    // for(const auto& [key, value] : obj){
    //     std::cout << key << " : " << *value << '\n';
    // }
    
    json["person"] += {{"hello", Json::Array({"How", "Are", "You"})}};
    json.write("testFile.json");
    std::cout << json << std::endl;

    return 0;
    }
