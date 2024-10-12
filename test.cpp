#include "jsonParse.hpp"
// #include <include/nlohmann/json.hpp>
#include <iostream>

int main(){
    Json json;
    // std::pair<std::string, std::string> test = {"name", "John Doe"};

    // json["person"] = Json::Object({
    //     {"name", "John Doe"},
    //     {"age", true},
    //     {"hobbies", Json::Array({
    //         {"running"},
    //         {"singing"},
    //         {"dancing"}
    //     })},
    //     {"address", Json::Object({
    //         {"street", "123 Main St"},
    //         {"city", "Somewhere"},
    //         {"zipcode", 12345},
    //         {"object", Json::Object({
    //             {"second_object", Json::Object({
    //                 {"third_object", "thank god"}
    //             })}
    //         })}
    //     })}});    
    json["test"] =
    Json::Object({
        {"testValue", Json::Null()},
        {"testNumber", true},
        {"testObject", Json::Object({
            {"testString", "Hello World\\ "}
        })},
        {"testValue3", 3.14}
    });


    json.write("testFile.json");
    JsonParse jsonParse;
    // Json json = jsonParse.parse("large-file.json");
    // Json json = jsonParse.parse("testFile.json");
    // std::cout << jsonParse.parse("testFile.json") << std::endl;

    // TODO
    // What the fuck
    // std::string testString = json["person"].asObject().at("address")->asObject().at("object")->asObject().at("second_object")->asObject().at("third_object")->getValue<std::string>();

    // std::cout << testString << std::endl;

    return 0;

    }
