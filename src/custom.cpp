#include "header.hpp"
#include <stdexcept>

namespace cats {
struct key {
    std::set<int> key_value;
    Json::Value joy_value;
    sf::Sprite sprite;
    bool status;
    double timer;

    key(Json::Value _key_value) {
        if (!_key_value.isMember("keyCodes") && !_key_value.isMember("joyCodes"))
            throw std::runtime_error("One of the fields: keyCodes or joyCodes must be present");

        if (_key_value.isMember("keyCodes")) {
            if (!_key_value["keyCodes"].isArray())
                throw std::runtime_error("Custom keyCodes values are not set correctly");
            key_value = data::json_key_to_scancodes(_key_value["keyCodes"]);
        }
        
        if (_key_value.isMember("joyCodes")) {
            if (!_key_value["joyCodes"].isArray())
                throw std::runtime_error("Custom joyCodes values is not set correctly");
            joy_value = _key_value["joyCodes"];
        }

        if (_key_value.isMember("image") && _key_value["image"].isString())
            sprite.setTexture(data::load_texture(_key_value["image"].asString()));
        else
            throw std::runtime_error("Custom image path is not set correctly");

        status = false;
        timer = -1;
    }

    bool is_pressed() {
        for (int v : key_value) {
            if (input::is_pressed(v)) {
                return true;
            }
        }

        if (input::is_joystick_connected()) {
            for (Json::Value &v : joy_value) {
                if (input::is_joystick_pressed(v.asInt())) {
                    return true;
                }
            }
        }

        return false;
    }

    void draw(sf::RenderTarget& window, sf::RenderStates rst) {
        window.draw(sprite, rst);
        timer = clock();
    }
};

struct key_container {
    std::vector<key> keys;
    sf::Sprite default_sprite;
    size_t key_index;

    key_container(Json::Value key_container_value) {
        if (!key_container_value.isObject())
            throw std::runtime_error("Key container must be an object");

        if (!key_container_value.isMember("defaultImage"))
            throw std::runtime_error("key container is missing property defaultImage");

        if (!key_container_value["defaultImage"].isString())
            throw std::runtime_error("defaultImage must be a string");

        if (key_container_value.isMember("keys")
            && !key_container_value["keys"].isArray())
            throw std::runtime_error("keys property must be an array");
        
        default_sprite.setTexture(data::load_texture(key_container_value["defaultImage"].asString()));
        for (const Json::Value &child_key : key_container_value["keys"]) {
            keys.push_back(key(child_key));
        }
    }

    void draw(sf::RenderTarget& window, sf::RenderStates rst) {
        bool is_any_key_pressed = false;
        for (size_t i = 0; i < keys.size(); i++) {
            key& current_key = keys[i];
            if (current_key.is_pressed()) {
                is_any_key_pressed = true;
                if (!current_key.status) {
                    key_index = i;
                    current_key.status = true;
                }
            } else {
                current_key.status = false;
            }
        }
        if (!is_any_key_pressed) {
            window.draw(default_sprite, rst);
        }
        else {
            key& on_key = keys[key_index];
            double last_press = -1;
            for (size_t i = 0; i < keys.size(); i++) {
                if (i != key_index) {
                    last_press = std::max(last_press, keys[i].timer);
                }
            }
            if ((clock() - last_press) / CLOCKS_PER_SEC > BONGO_KEYPRESS_THRESHOLD) {
                on_key.draw(window, rst);
            } else {
                window.draw(default_sprite, rst);
            }
        }
    }
};

std::vector<key_container> key_containers;

bool CustomCat::init(const data::Settings& st, const Json::Value& config) {
    // getting configs
    try {
        key_containers.clear();
        for (const Json::Value& current_key_container : config["keyContainers"]) {
            key_containers.push_back(key_container(current_key_container));
        }
        if (!config.isMember("background") || !config["background"].isString())
            throw std::runtime_error("Custom background not found");

        bg.setTexture(data::load_texture(config["background"].asString()));
        
        is_mouse = config["mouse"].asBool();
        if (is_mouse) {
            is_mouse_on_top = config["mouseOnTop"].asBool();
            
            sf::Vector2i offset;
            offset.x = config["offsetX"].asInt();
            offset.y = config["offsetY"].asInt();
            double scale = config["scalar"].asDouble();
            MousePaw::set_mouse_parameters(offset, scale);

            if (!config.isMember("mouseImage") || !config["mouseImage"].isString())
                throw std::runtime_error("Mouse image not found");

            device.setTexture(data::load_texture(config["mouseImage"].asString()));
        }

        MousePaw::init(config, st.get_global_mouse_config());
    } catch (std::runtime_error& e) {
        logger::error(std::string("Config error: ") + e.what());
        return false;
    }
    return true;
}

void CustomCat::update() {
    if (is_mouse) {
        // update mouse and paw position
        update_paw_position(input::get_mouse_input().get_position());
    }
}

void CustomCat::draw(sf::RenderTarget& target, sf::RenderStates rst) const {
    target.draw(bg, rst);

    if (is_mouse) {
        // drawing mouse on top
        if (is_mouse_on_top) {
            target.draw(device, rst);
        }

        // draw mouse paw
        draw_paw(target, rst);
    }

    for (key_container& current : key_containers) {
        current.draw(target, rst);
    }

    // drawing mouse at the bottom
    if (is_mouse && !is_mouse_on_top) {
        target.draw(device, rst);
    }
}

} // namespace cats
