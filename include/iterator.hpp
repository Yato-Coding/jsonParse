#ifndef JSON_ITERATOR_HPP
#define JSON_ITERATOR_HPP

#include <memory>
#include <variant>
#include <vector>
#include <unordered_map>


class JsonValue;
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


#endif