#include <json/json.h>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Transform.hpp>

#include <optional>
#include <stdexcept>

namespace data {

class Settings {
public:
    bool reload();

    // window settings
    sf::Vector2i get_window_size() const;
    sf::Transform get_window_transform() const;

    // mouse settings
    bool is_mouse_left_handed() const;
    const Json::Value get_global_mouse_config() const;

    // decoration settings
    sf::Color get_background_color() const;
    sf::Vector2i get_offset(bool is_mouse) const;
    double get_scale(bool is_mouse) const;

    // cats' settings
    std::string get_default_mode() const;
    const Json::Value& get_cat_config(const std::string& name) const;
    std::vector<std::string> get_cat_modes() const;

private:
    bool find_cat_modes(const Json::Value &cfg);

    Json::Value config;
    std::vector<std::string> modes;
};

namespace detail {

template<typename T> T validate(const Json::Value& property);
template<> bool validate<bool>(const Json::Value& property);
template<> double validate<double>(const Json::Value& property);
template<> std::string validate<std::string>(const Json::Value& property);
template<> sf::Vector2i validate<sf::Vector2i>(const Json::Value& property);
template<> sf::Color validate<sf::Color>(const Json::Value& property);

}

class Validator {
public:
    Validator(const Json::Value& section)
        : m_Section(section) {}

    template<typename T>
    std::optional<T> getProperty(const std::string& name) {
        if (m_Section.isMember(name)) {
            try {
                return detail::validate<T>(m_Section[name]);
            }
            catch(std::runtime_error& e) {
                throw std::runtime_error(
                    "Invaild property " + name + ": " + e.what());
            }
        }
        else
            return std::nullopt;
    }

    template<typename T>
    T getProperty(const std::string& name, T default_value) {
        return getProperty<T>(name).value_or(default_value);
    }

private:
    const Json::Value& m_Section;
};

}