#include "header.hpp"
#include <json/value.h>
#include <memory>
#include <system.hpp>
#include <filesystem>
#include <algorithm>
#include <optional>

#include <unistd.h>
#include <limits.h>

namespace data {
std::unique_ptr<sf::Font> debug_font_holder;
std::map<std::string, sf::Texture> img_holder;

template<class C, class T>
bool contains(C container, T object) {
    return std::find(container.begin(), container.end(), object) != std::end(container);
}

const sf::Vector2i g_window_default_size(612, 352);

static inline sf::Vector2i value_or(const Json::Value& v, sf::Vector2i defv) {
    return sf::Vector2i(
        v[0].isNull() ? defv.x : v[0].asInt(),
        v[1].isNull() ? defv.y : v[1].asInt()
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

sf::Vector2i Settings::get_window_size() const {
    return value_or(config["window"]["size"], g_window_default_size);
}

sf::Transform Settings::get_window_transform() const {
    auto window_config = config["window"];
    const sf::Vector2i window_size = get_window_size();
    const sf::Vector2i window_offset = value_or(window_config["offset"], sf::Vector2i(0, 0));

    sf::Vector2f scene_pos;
    scene_pos.x = std::clamp(window_offset.x, 0, window_size.x);
    scene_pos.y = std::clamp(window_offset.y, 0, window_size.y);

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

const Json::Value Settings::get_global_mouse_config() const {
    return config["mousePaw"];
}

sf::Color Settings::get_background_color() const {
    Json::Value rgb = config["decoration"]["rgb"];
    int red_value = rgb[0].asInt();
    int green_value = rgb[1].asInt();
    int blue_value = rgb[2].asInt();
    int alpha_value = rgb.size() == 3 ? 255 : rgb[3].asInt();

    return sf::Color(red_value, green_value, blue_value, alpha_value);
}

sf::Vector2i Settings::get_offset(bool is_mouse) const {
    sf::Vector2i offset;

    if (is_mouse) {
        offset.x = (config["decoration"]["offsetX"])[0].asInt();
        offset.y = (config["decoration"]["offsetY"])[0].asInt();
    } else {
        offset.x = (config["decoration"]["offsetX"])[1].asInt();
        offset.y = (config["decoration"]["offsetY"])[1].asInt();
    }

    return offset;
}

double Settings::get_scale(bool is_mouse) const {
    return is_mouse
        ? (config["decoration"]["scalar"])[0].asDouble()
        : (config["decoration"]["scalar"])[1].asDouble();
}

std::string Settings::get_default_mode() const {
    return config["mode"].asString();
}

const Json::Value& Settings::get_cat_config(const std::string& name) const{
    return config.isMember(name) ? config[name] : config["modes"][name];
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

std::set<int> json_key_to_scancodes(const Json::Value& key) {
    std::set<int> codes;

    if (key.isArray()) {
        for (const Json::Value &v : key) {
            auto code = json_key_to_scancode(v);
            if (code.has_value()) {
                codes.insert(*code);
            }
        }
    }
    else {
        auto code = json_key_to_scancode(key);
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

static bool update(Json::Value &cfg_default, const Json::Value &cfg) {
    bool is_update = true;
    
    for (const auto &key : cfg.getMemberNames()) {
        if (cfg_default.isMember(key)) {
            if (cfg_default[key].type() != cfg[key].type()) {
                logger::error("Error in " CONF_FILE_NAME ": Value type error in ");
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
                            logger::error("Error in " CONF_FILE_NAME ": Value type error in array ");
                            return false;
                        }
                    }
                }
            }
            if (cfg_default[key].isObject()) {
                is_update &= update(cfg_default[key], cfg[key]);
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

bool init() {
    // load debug font
    debug_font_holder = std::make_unique<sf::Font>();
    if (!debug_font_holder->loadFromFile("share/RobotoMono-Bold.ttf")) {
        logger::error("Error loading font: Cannot find the font : RobotoMono-Bold.ttf");
        return false;
    }

    img_holder.clear();
    return true;
}

bool Settings::reload() {
    const auto system_info = os::create_system_info();

    std::string conf_file_path = CONF_FILE_NAME;
    // if a file exists in the current directory it takes precendence
    if(!std::filesystem::exists(conf_file_path)) {
        // otherwise try to load a file from user's home directory
        auto cfg_dir_path = system_info->get_config_dir_path();
        conf_file_path = cfg_dir_path + CONF_FILE_NAME;
        if(!std::filesystem::exists(conf_file_path)) {
            // if no config file is present, create one with the default settings
            const std::string cfg_file_template_path = "share/" CONF_FILE_NAME;
                
            if (std::filesystem::exists(cfg_file_template_path)) {
                // create config from the default config template
                std::filesystem::create_directories(cfg_dir_path);
                std::filesystem::copy(cfg_file_template_path, conf_file_path);
            }
        }
    }
    std::ifstream cfg_file(conf_file_path);
    if (!cfg_file.good()) {
        std::string msg = "Error reading configs: Couldn't open config file " 
            + conf_file_path + ":\n";
        logger::error(msg);
        return false;
    }

    std::unique_ptr<Json::Value> cfg_read;
    try {
        cfg_read = parse_config_file(cfg_file);
    }
    catch(std::runtime_error& e) {
        std::string msg = "Error reading configs: Syntax error in " CONF_FILE_NAME ":\n";
        logger::error( msg + e.what());
        return false;
    }

    if (!update(config, *cfg_read))
        return false;

    return find_cat_modes(config);
}

sf::Texture &load_texture(std::string path) {
    if (img_holder.find(path) == img_holder.end()) {
        while (!img_holder[path].loadFromFile(path)) {
            logger::error("Error importing images: Cannot find file " + path);
        }
    }
    return img_holder[path];
}

sf::Font &get_debug_font() {
    return *debug_font_holder;
}

}; // namespace data
