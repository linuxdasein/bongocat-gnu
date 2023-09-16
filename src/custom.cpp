#include "header.hpp"

namespace cats {
struct key {
    Json::Value key_value;
    Json::Value joy_value;
    sf::Sprite sprite;
    bool status;
    double timer;

    key(Json::Value _key_value) {
        sprite = sf::Sprite();
        if (_key_value.isMember("keyCodes") && _key_value["keyCodes"].isArray()) {
            key_value = _key_value["keyCodes"];
        } else {
            data::error_msg("Custom keyCodes values is not set correctly", "Error reading configs");
            throw;
        }
        if (_key_value.isMember("image") && _key_value["image"].isString()) {
            sprite = sf::Sprite();
            sprite.setTexture(data::load_texture(_key_value["image"].asString()));
        } else {
            data::error_msg("Custom image path is not set correctly", "Error reading configs");
            throw;
        }
        if (_key_value.isMember("joyCodes")) {
            if (!_key_value["joyCodes"].isArray()) {
                data::error_msg("Custom joyCodes values is not set correctly", "Error reading configs");
                throw;
            }
            joy_value = _key_value["joyCodes"];
        }
        status = false;
        timer = -1;
    }

    bool is_pressed() {
        for (Json::Value &v : key_value) {
            if (input::is_pressed(v.asInt())) {
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
        if (key_container_value.isObject()) {
            if (!key_container_value.isMember("defaultImage")
                || !key_container_value["defaultImage"].isString()
                || !key_container_value.isMember("keys")
                || !key_container_value["keys"].isArray()) {
                data::error_msg("Key container's object error", "Error reading configs");
                throw;
            } else {
                default_sprite = sf::Sprite();
                default_sprite.setTexture(data::load_texture(key_container_value["defaultImage"].asString()));
                for (Json::Value &child_key : key_container_value["keys"]) {
                    keys.push_back(key(child_key));
                }
            }
        } else {
            data::error_msg("Key container must be an object", "Error reading configs");
            throw;
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

bool CustomCat::init(const Json::Value& cfg) {
    // getting configs
    try {
        Json::Value custom = cfg["custom"];
        key_containers.clear();
        for (Json::Value& current_key_container : custom["keyContainers"]) {
            key_containers.push_back(key_container(current_key_container));
        }
        if (!custom.isMember("background") || !custom["background"].isString()) {
            data::error_msg("Custom background not found", "Error reading config");
            return false;
        }
        bg.setTexture(data::load_texture(custom["background"].asString()));
        
        is_mouse = custom["mouse"].asBool();
        if (is_mouse) {
            is_mouse_on_top = custom["mouseOnTop"].asBool();
            
            sf::Vector2i offset;
            offset.x = custom["offsetX"].asInt();
            offset.y = custom["offsetY"].asInt();
            double scale = custom["scalar"].asDouble();
            MousePaw::set_mouse_parameters(offset, scale);

            if (!custom.isMember("mouseImage") || !custom["mouseImage"].isString()) {
                data::error_msg("Mouse image not found", "Error reading config");
                return false;
            }
            device.setTexture(data::load_texture(custom["mouseImage"].asString()));
        }

        MousePaw::init(custom, cfg["mousePaw"]);
    } catch (...) {
        return false;
    }
    return true;
}

void CustomCat::draw(sf::RenderTarget& window, sf::RenderStates rst) {
    window.draw(bg, rst);

    if (is_mouse) {
        // update mouse and paw position
        auto pss2 = update_paw_position(input::get_mouse_input().get_position());

        // drawing mouse on top
        if (is_mouse_on_top) {
            window.draw(device, rst);
        }

        // draw mouse paw
        draw_paw(window, pss2, rst);
    }

    for (key_container& current : key_containers) {
        current.draw(window, rst);
    }

    // drawing mouse at the bottom
    if (is_mouse && !is_mouse_on_top) {
        window.draw(device, rst);
    }
}

} // namespace cats
