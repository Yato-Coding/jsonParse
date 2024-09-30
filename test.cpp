#include "jsonParse.hpp"
#include <iostream>

int main(){
    Json json;
    json["person"] =
        Json::Object({
        {"name", "John Doe"},
        {"age", 30},
        {"hobbies", Json::Array({
            "running",
            "singing",
            "dancing",
        })},
        {"address", Json::Object({
            {"street", "123 Main St"},
            {"city", "Somewhere"},
            {"zipcode", 12345}
        })}
    });
    json["test"] =
        Json::Object({
            {"testValue", Json::Null()},
            {"testValue2", 42},
            {"testValue3", 3.14}
        });

    // std::cout << json << std::endl;
    json.write("testFile");
    json.parse("testFile");

    return 0;
}
