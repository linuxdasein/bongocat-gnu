#include "header.hpp"
#include <SFML/System/Vector2.hpp>
#include <stdexcept>

namespace {

template<typename T, typename P>
void move_if(std::list<T>& dst, std::list<T>& src, P condition) {
    for( auto it = src.begin(); it != src.end(); ) {
        // store iterator value before increment 
        // since it will point to the other list after splicing
        auto tmp = it++; 
        if(condition(*tmp)) {
            dst.splice(dst.end(), src, tmp);
        }
    }
}

class SpriteArray : public sf::Drawable {
public:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        for (const auto& s : sprites)
            target.draw(s, states);
    }

    void add(sf::Sprite sprite) {
        sprites.push_back(sprite);
    }

private:
    std::vector<sf::Sprite> sprites;
};

}

namespace cats {

void CatKeyboardGroup::init(const Json::Value& keys_config) {
    if (keys_config.isMember("defaultImages")) {
        if (!keys_config["defaultImages"].isArray())
            throw std::runtime_error("defaultImages must be an array");
        auto sprites = std::make_unique<SpriteArray>();
        for (const auto& image : keys_config["defaultImages"])
            sprites->add(sf::Sprite(data::load_texture(image.asString())));
        def_kbg = std::move(sprites);
    }

    if (!keys_config.isMember("keyBindings"))
        return;

    if (!keys_config["keyBindings"].isArray())
        throw std::runtime_error("keyBindings must be an array");

    for (const auto& binding : keys_config["keyBindings"]) {
        if (!binding.isObject())
            throw std::runtime_error("invalid property in keyBindings array, an object is expected");

        if (!binding.isMember("images"))
            throw std::runtime_error("invalid object in keyBindings: \
                                      an object must contain field images");

        if (!binding.isMember("keyCodes") && !binding.isMember("joyCodes"))
            throw std::runtime_error("invalid object in keyBindings: "
                                     "an object must contain a keyCodes or a joyCodes field");

        bool is_persistent = false;
        if (binding.isMember("isPersistent"))
            is_persistent = binding["isPersistent"].asBool();

        auto sprites = std::make_unique<SpriteArray>();
        for (const auto& image : binding["images"])
            sprites->add(sf::Sprite(data::load_texture(image.asString())));

        auto key_codes = data::json_key_to_scancodes(binding["keyCodes"]);

        for (auto key_code : key_codes) {
            released_keys.push_back(Key{key_code, is_persistent});
            key_actions[key_code] = std::move(sprites);
        }
    }
}

void CatKeyboardGroup::update() {
    // Update states for the keys which currently are not pressed down
    move_if(pressed_keys, released_keys, 
        [&](Key key){ return input::is_pressed(key.code); });
    // Some key bindings are marked as pesistent, store them separately
    move_if(persistent_keys, pressed_keys, 
        [&](Key key){ return key.is_persistent; });
    // Update states for pressed keys which have been released
    move_if(released_keys, pressed_keys, 
        [&](Key key){ return !input::is_pressed(key.code); });
    // Update states for pesistent keys which have been released
    move_if(released_keys, persistent_keys, 
        [&](Key key){ return !input::is_pressed(key.code); });
}
    
void CatKeyboardGroup::draw(sf::RenderTarget& target, sf::RenderStates rst) const {
    // draw persistent bindings
    for (auto pk : persistent_keys) {
        target.draw(*key_actions.at(pk.code), rst);
    }

    if(pressed_keys.empty()) {
        target.draw(*def_kbg, rst);
    }
    else {
        // draw the latest pressed key sprite
        target.draw(*key_actions.at(pressed_keys.back().code), rst);
    }
}

bool CustomCat::init(const data::Settings&, const Json::Value& config) {
    // getting configs
    try {
        kbd_groups.clear();
        if (!config.isMember("background") || !config["background"].isString())
            throw std::runtime_error("Custom background not found");

        bg.setTexture(data::load_texture(config["background"].asString()));

        if (config.isMember("keyboard")) {
            if(config["keyboard"].isArray()){
                for(auto kbd_section : config["keyboard"]) {
                    auto kbind = std::make_unique<CatKeyboardGroup>();
                    kbind->init(kbd_section);
                    kbd_groups.push_back(std::move(kbind));
                }
            }
            else {
                auto kbind = std::make_unique<CatKeyboardGroup>();
                kbind->init(config["keyboard"]);
                kbd_groups.push_back(std::move(kbind));
            }
        }
        
        if (config.isMember("mouse")) {
            is_mouse = init_mouse(config["mouse"]);
        }
        else {
            is_mouse = false;
            logger::info("No mouse property found in cat's config section, \
                          assuming mouse is disabled");
        }
    } catch (std::runtime_error& e) {
        logger::error(std::string("Config error: ") + e.what());
        return false;
    }
    return true;
}

bool CustomCat::init_mouse(const Json::Value& config) {
    data::Validator cfg(config);
    const auto offset = cfg.getProperty("offset", sf::Vector2i(0, 0));
    const auto scale = cfg.getProperty("scale", 1.0);

    if (scale <= 0) {
        throw std::runtime_error("Invalid option value: scale = " 
            + std::to_string(scale) + ". A positive value is expected");
    }

    const auto image_path = cfg.getProperty<std::string>("image");
    is_mouse_on_top = cfg.getProperty("isOnTop", false);

    if (!image_path)
        throw std::runtime_error("No image is set in mouse config section");

    device.setTexture(data::load_texture(image_path.value()));
    MousePaw::set_mouse_parameters(offset, scale);

    if (config.isMember("buttons")) {
        data::Validator key_bindings(config["buttons"]);
        const auto lb_image_path = key_bindings.getProperty<std::string>("left");
        const auto rb_image_path = key_bindings.getProperty<std::string>("right");

        if (lb_image_path)
            left_button.setTexture(data::load_texture(lb_image_path.value()));
        if (rb_image_path)
            right_button.setTexture(data::load_texture(rb_image_path.value()));
    }

    if (!MousePaw::init(config, config))
        throw std::runtime_error("Failed to initialize mouse paw");

    return cfg.getProperty("isEnabled", true);
}

void CustomCat::update() {
    if (is_mouse) {
        // update mouse and paw position
        update_paw_position(input::get_mouse_input().get_position());
    }

    for (auto& kbd_group : kbd_groups)
        kbd_group->update();
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

    // draw keyboard bindings
    for (auto& kbd_group : kbd_groups)
        kbd_group->draw(target, rst);

    // drawing mouse at the bottom
    if (is_mouse && !is_mouse_on_top) {
        target.draw(device, rst);
    }

    // draw mouse buttons
    if(input::get_mouse_input().is_left_button_pressed())
        target.draw(left_button, rst);
    if(input::get_mouse_input().is_right_button_pressed())
        target.draw(right_button, rst);
}

} // namespace cats
