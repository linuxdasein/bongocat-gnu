#include <json/json.h>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Transform.hpp>

#include <optional>
#include <stdexcept>
#include <fstream>

namespace data {

class ConfigFile {
public:
    ConfigFile();

    bool init(int argc, char ** argv);

    std::ifstream& load_config_file();
    std::string get_config_name() const;

private:
    std::ifstream cfg_file;
    std::string conf_file_path;
};

class Settings {
public:
    bool reload(ConfigFile &cfg_file);

    // window settings
    sf::Vector2i get_window_size() const;
    sf::Transform get_window_transform() const;

    // global mouse settings
    bool is_mouse_left_handed() const;

    // global decoration settings
    sf::Color get_background_color() const;

    // cats' settings
    std::string get_default_mode() const;
    const Json::Value& get_cat_config(const std::string& name) const;
    std::vector<std::string> get_cat_modes() const;

private:
    bool find_cat_modes(const Json::Value &cfg);
    bool check_config_version(const Json::Value &cfg, std::string min_required);

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
    Validator(const Json::Value& section);

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