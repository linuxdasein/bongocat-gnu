#include <header.hpp>
#include <json/value.h>
#include <memory>
#include <stdexcept>
#include <system.hpp>
#include <algorithm>
#include <filesystem>
#include <optional>
#include <sstream> 

#include <unistd.h>

namespace data {
std::unique_ptr<sf::Font> debug_font_holder;
std::map<std::string, sf::Texture> img_holder;

template<class C, class T>
bool contains(C container, T object) {
    return std::find(container.begin(), container.end(), object) != std::end(container);
}

const sf::Vector2u g_window_default_size(612, 352);

static inline sf::Vector2u value_or(const Json::Value& v, sf::Vector2u defv) {
    return sf::Vector2u(
        v[0].isNull() ? defv.x : v[0].asUInt(),
        v[1].isNull() ? defv.y : v[1].asUInt()
    );
}

std::unique_ptr<Json::Value> parse_config_file(std::ifstream& cfg_file) {
    std::string cfg_string((std::istreambuf_iterator<char>(cfg_file)), std::istreambuf_iterator<char>()), error;
    Json::CharReaderBuilder cfg_builder;
    auto cfg = std::make_unique<Json::Value>();
    Json::CharReader *cfg_reader = cfg_builder.newCharReader();

    if (!cfg_reader->parse(cfg_string.c_str(), cfg_string.c_str() + cfg_string.size(), cfg.get(), &error)) {
        delete cfg_reader;
        throw std::runtime_error(error);
    } 
        
    delete cfg_reader;

    return cfg;
}

sf::Vector2u Settings::get_window_size() const {
    return value_or(config["window"]["size"], g_window_default_size);
}

sf::Transform Settings::get_window_transform() const {
    auto window_config = config["window"];
    const sf::Vector2u window_size = get_window_size();
    const sf::Vector2u window_offset = value_or(window_config["offset"], sf::Vector2u(0, 0));

    sf::Vector2f scene_pos;
    scene_pos.x = std::clamp(window_offset.x, 0u, window_size.x);
    scene_pos.y = std::clamp(window_offset.y, 0u, window_size.y);

    const bool is_adaptive = window_config["adaptive"].isNull() ?
        false : window_config["adaptive"].asBool();

    const float cfg_scale = window_config["scale"].isNull() ?
        1.0f : window_config["scale"].asFloat();

    sf::Transform transform = sf::Transform();
    transform.translate(scene_pos);
    transform.scale(sf::Vector2f(cfg_scale, cfg_scale));

    if(is_adaptive) {
        sf::Vector2f relative_scale;
        relative_scale.x = float(window_size.x - window_offset.x) / g_window_default_size.x;
        relative_scale.y = float(window_size.y - window_offset.y) / g_window_default_size.y;
        const float min_scale = std::min(relative_scale.x, relative_scale.y);
        transform.scale(sf::Vector2f(min_scale, min_scale));
    }

    return transform;
}

bool Settings::is_mouse_left_handed() const {
    return config["decoration"]["leftHanded"].asBool();
}

sf::Color Settings::get_background_color() const {
    Json::Value rgb = config["decoration"]["rgb"];
    int red_value = rgb[0].asInt();
    int green_value = rgb[1].asInt();
    int blue_value = rgb[2].asInt();
    int alpha_value = rgb.size() == 3 ? 255 : rgb[3].asInt();

    return sf::Color(red_value, green_value, blue_value, alpha_value);
}

std::string Settings::get_default_mode() const {
    return config["mode"].asString();
}

const Json::Value& Settings::get_cat_config(const std::string& name) const{
    return config["modes"][name];
}

std::vector<std::string> Settings::get_cat_modes() const {
    return modes;
}

std::optional<int> json_key_to_scancode(const Json::Value& key) {
    if (key.isInt()) {
        return key.asInt();
    }
    else if (key.isString()) {
        std::string s = key.asString();
        if (s.size() != 1) {
            logger::error("Error reading configs: Invalid key value: " + s);
        }
        else {
            // treat uppercase and lowercase letters equally
            char c = std::toupper(s[0]);
            return static_cast<int>(c);
        }
    }
    else {
        logger::error("Error reading configs: Invalid key value: " + key.asString());
    }
    return std::nullopt;
}

std::optional<int> json_joy_key_to_scancode(const Json::Value& key) {
    if (key.isInt()) {
        return key.asInt();
    }
    else {
        logger::error("Error reading configs: Invalid key value: " + key.asString());
    }
    return std::nullopt;
}

std::set<int> json_key_to_scancodes(const Json::Value& key, bool is_joystick) {
    std::set<int> codes;

    if (key.isArray()) {
        for (const Json::Value &v : key) {
            auto code = is_joystick 
                ? json_joy_key_to_scancode(v) 
                : json_key_to_scancode(v);
            if (code.has_value()) {
                codes.insert(*code);
            }
        }
    }
    else {
        auto code = is_joystick 
            ? json_joy_key_to_scancode(key) 
            : json_key_to_scancode(key);
        if (code.has_value()) {
            codes.insert(*code);
        }
    }

    return codes;
}

bool is_intersection(const std::vector<std::set<int>>& sets) {
    // the algorithm here is meant to work with ordered sets
    std::set<int> u = sets[0];

    for(size_t i = 1; i < sets.size(); ++i) {
        std::set<int> tmp;
        auto s1 = u.cbegin();
        auto s2 = sets[i].cbegin();
        while( s1 != u.cend() && s2 != sets[i].cend()) {
            if(*s1 < *s2) {
                tmp.insert(*s1++);
            }
            else if(*s1 > *s2) {
                tmp.insert(*s2++);
            }
            else {
                // *s1 == *s2
                return true;
            }
        }
        u.merge(tmp);
        while(s1 != u.cend())
            u.insert(*s1++);
        while(s2 != sets[i].cend())
            u.insert(*s2++);
    }

    return false;
}

static bool update(Json::Value &cfg_default, const Json::Value &cfg, const std::string &cfg_name) {
    bool is_update = true;
    
    for (const auto &key : cfg.getMemberNames()) {
        if (cfg_default.isMember(key)) {
            if (cfg_default[key].type() != cfg[key].type()) {
                logger::error("Error in " + cfg_name + ": Value type error in ");
                return false;
            }
            if (cfg_default[key].isArray() && !cfg_default[key].empty()) {
                for (const Json::Value &v : cfg[key]) {
                    auto new_v_type = cfg_default[key][0].type();
                    if (v.type() != new_v_type) {
                        // explicitly allow to chenge int to string and other way around
                        // since these types are used to represent key codes
                        const auto interchangeable_types = {Json::ValueType::intValue, 
                                                            Json::ValueType::stringValue};
                        const bool is_interchange_allowed = 
                            contains(interchangeable_types, v.type())
                            && contains(interchangeable_types, new_v_type);
                        if(!is_interchange_allowed) {
                            logger::error("Error in " + cfg_name + ": Value type error in array ");
                            return false;
                        }
                    }
                }
            }
            if (cfg_default[key].isObject()) {
                is_update &= update(cfg_default[key], cfg[key], cfg_name);
            } else {
                cfg_default[key] = cfg[key];
            }
        } else {
            cfg_default[key] = cfg[key];
        }
    }
    return is_update;
}

bool Settings::find_cat_modes(const Json::Value &cfg) {

    if (!cfg.isMember("modes")) {
        logger::error("No modes entry is found in config file");
        return false;
    }

    for (const auto& m : cfg["modes"].getMemberNames()) {
        modes.push_back(m);
    }

    return true;
}

bool Settings::check_config_version(const Json::Value &cfg, std::string min_required) {
    if (!cfg.isMember("version") || !cfg["version"].isString())
        return false;

    auto parse_version = [](std::string s) {
        std::string tk;
        std::vector<int> vnum;
        std::stringstream vstr(s);

        while (std::getline(vstr, tk, '.'))
            vnum.push_back(std::stoi(tk));
        return vnum;
    };

    std::vector<int> required_version, config_version;

    try {
        required_version = parse_version(min_required);
        config_version = parse_version(cfg["version"].asString());
    }
    catch(const std::invalid_argument& e) {
        return false;
    }

    if (required_version.size() != config_version.size())
        return false;

    for (size_t i = 0; i < config_version.size(); ++i) {
        if (config_version[i] < required_version[i])
            return false;
    }

    return true;
}

bool init() {
    // load debug font
    debug_font_holder = std::make_unique<sf::Font>();

    const auto system_info = os::create_system_info();
    const std::string local_path = "share/RobotoMono-Bold.ttf";
    const std::string full_path = system_info->get_app_dir_path() / local_path;

    if (!std::filesystem::exists(full_path) || !debug_font_holder->openFromFile(full_path)) {
        // if not found in install prefix, try current directory
        if (!std::filesystem::exists(local_path) || !debug_font_holder->openFromFile(local_path)) {
            logger::error("Error loading font: Cannot find the font : RobotoMono-Bold.ttf");
            return false;
        } 
    }

    img_holder.clear();
    return true;
}

bool Settings::reload(ConfigFile &cfg_file) {
    std::unique_ptr<Json::Value> cfg_read;
    
    try {
        cfg_read = parse_config_file(cfg_file.load_config_file());
    }
    catch(std::runtime_error& e) {
        std::string msg = "Error reading config " + cfg_file.get_config_name() + ":\n";
        logger::error( msg + e.what());
        return false;
    }

    if (!update(config, *cfg_read, cfg_file.get_config_name()))
        return false;

    const std::string min_version = "0.3.0";
    if (!check_config_version(config, min_version)) {
        logger::error( "Required config version>=" + min_version +
                       ". You have to update your config file manually." );
        return false;
    }

    return find_cat_modes(config);
}

sf::Texture &load_texture(std::string path) {
    bool is_full_path = path[0] == '/';
    const auto system_info = os::create_system_info();
    std::filesystem::path full_path = is_full_path 
        ? std::filesystem::path(path)
        : system_info->get_app_dir_path() / path;

    if (img_holder.find(full_path) != img_holder.end())
        return img_holder[full_path];

    if (img_holder.find(path) != img_holder.end())
        return img_holder[path];

    if (std::filesystem::exists(full_path)
        && img_holder[full_path].loadFromFile(full_path))
        return img_holder[full_path];

    if (std::filesystem::exists(path)
        && img_holder[path].loadFromFile(path))
        return img_holder[path];
        
    throw std::runtime_error("Error importing images: Cannot open file " + full_path.string());
}

sf::Font &get_debug_font() {
    return *debug_font_holder;
}

}; // namespace data
