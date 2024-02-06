#include "data.hpp"

namespace data {
namespace detail {

template<> bool validate<bool>(const Json::Value& property) {
    if (property.isBool()) 
        return property.asBool();
    else
        throw std::runtime_error("A boolean value is expected");
}

template<> double validate<double>(const Json::Value& property) {
    if (property.isDouble()) 
        return property.asDouble();
    else
        throw std::runtime_error("A floating point value is expected");
}

template<> std::string validate<std::string>(const Json::Value& property) {
    if (property.isString())
        return property.asString();
    else
        throw std::runtime_error("A string value is expected");
}

template<> sf::Vector2i validate<sf::Vector2i>(const Json::Value& property) {
    if (property.isArray()) {
        if (property.size() != 2)
            throw std::runtime_error(
                "Invalid array size: an array of size 2 is expected");
        
        for(const auto& elem : property) {
            if (!elem.isInt())
                throw std::runtime_error("Array has an element of an invalid type; \
                                          An integer value is expected");
        }

        return sf::Vector2i(property[0].asInt(), property[1].asInt());
    } else
        throw std::runtime_error("An array is expected");
}

template<> sf::Color validate<sf::Color>(const Json::Value& property) {
    if (!property.isArray())
        throw std::runtime_error("An array is expected");

    for(const auto& elem : property) {
        if (!elem.isInt())
            throw std::runtime_error("Array has an element of an invalid type; \
                                      An integer value is expected");
    }

    if (property.size() == 3)
        return sf::Color(property[0].asInt(), property[1].asInt(), property[2].asInt());
    else if (property.size() == 4)
        return sf::Color(property[0].asInt(), property[1].asInt(), property[2].asInt(), property[3].asInt());
    else
        throw std::runtime_error("Invalid array size. The array must contain 3 or 4 values");
}

}
}