#include <json/json.h>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Transform.hpp>

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

}