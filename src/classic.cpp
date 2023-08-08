#include "header.hpp"
#include <json/json.h>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <array>

namespace {

class PawAction : public sf::Drawable {
public:
    PawAction(sf::Sprite&& p, sf::Sprite&& b)
        : paw(p), button(b) {}

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        target.draw(button, states);
        target.draw(paw, states);
    }

private:
    sf::Sprite paw;
    sf::Sprite button;
};

struct KeyActionData {
    sf::Keyboard::Key key_code;
    std::string key_texture;
    std::string paw_texture;
};

const std::array<KeyActionData, 15> actions_data = {
    KeyActionData{sf::Keyboard::Num1,  "img/classic/keyboard/0.png",  "img/classic/lefthand/0.png"},
    KeyActionData{sf::Keyboard::Num2,  "img/classic/keyboard/1.png",  "img/classic/lefthand/1.png"},
    KeyActionData{sf::Keyboard::Num3,  "img/classic/keyboard/2.png",  "img/classic/lefthand/2.png"},
    KeyActionData{sf::Keyboard::Num4,  "img/classic/keyboard/3.png",  "img/classic/lefthand/3.png"},
    KeyActionData{sf::Keyboard::Num5,  "img/classic/keyboard/4.png",  "img/classic/lefthand/4.png"},
    KeyActionData{sf::Keyboard::Num6,  "img/classic/keyboard/5.png",  "img/classic/lefthand/5.png"},
    KeyActionData{sf::Keyboard::Num7,  "img/classic/keyboard/6.png",  "img/classic/lefthand/6.png"},
    KeyActionData{sf::Keyboard::Q,     "img/classic/keyboard/7.png",  "img/classic/lefthand/7.png"},
    KeyActionData{sf::Keyboard::E,     "img/classic/keyboard/8.png",  "img/classic/lefthand/8.png"},
    KeyActionData{sf::Keyboard::R,     "img/classic/keyboard/9.png",  "img/classic/lefthand/9.png"},
    KeyActionData{sf::Keyboard::Space, "img/classic/keyboard/10.png", "img/classic/lefthand/10.png"},
    KeyActionData{sf::Keyboard::A,     "img/classic/keyboard/11.png", "img/classic/lefthand/11.png"},
    KeyActionData{sf::Keyboard::D,     "img/classic/keyboard/12.png", "img/classic/lefthand/12.png"},
    KeyActionData{sf::Keyboard::S,     "img/classic/keyboard/13.png", "img/classic/lefthand/13.png"},
    KeyActionData{sf::Keyboard::W,     "img/classic/keyboard/14.png", "img/classic/lefthand/14.png"},
};

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

}

namespace cats {

bool ClassicCat::init(const Json::Value& cfg) {

    cat.setTexture(data::load_texture("img/classic/catbg.png"));
    left_paw.setTexture(data::load_texture("img/classic/lefthand/leftup.png"));
    mouse.setTexture(data::load_texture("img/osu/mouse.png"), true);
    mouse.setScale(1.0, 1.0f);

    for( const auto& key: actions_data ) {
        key_actions[key.key_code] = std::make_unique<PawAction>(
            sf::Sprite(data::load_texture(key.paw_texture)),
            sf::Sprite(data::load_texture(key.key_texture))
        );
        keys.push_back(key.key_code);
    }

    int offset_x = 0, offset_y = 0, scale = 1;
    offset_x = (cfg["decoration"]["offsetX"])[0].asInt();
    offset_y = (cfg["decoration"]["offsetY"])[0].asInt();
    scale = (cfg["decoration"]["scalar"])[0].asDouble();

    // for this mode use separate paw adjustments from the corresponding section
    Json::Value cfg_std = cfg["classic"];

    device.setTexture(data::load_texture("img/osu/mouse.png"), true);
    MousePaw::init(cfg_std, cfg_std, offset_x, offset_y, scale);

    return true;
}

void ClassicCat::draw(sf::RenderWindow& window) {
    window.draw(cat);
    draw_mouse(window);

    // First, update states for the keys which currently are not pressed down
    move_if(pressed_keys, keys, 
        [&](sf::Keyboard::Key key){ return sf::Keyboard::isKeyPressed(key); });

    if(pressed_keys.empty())
        window.draw(left_paw);
    else // draw the latest pressed key sprite
        window.draw(*key_actions[pressed_keys.back()]);

    // Update states for the keys which have been released
    move_if(keys, pressed_keys, 
        [&](sf::Keyboard::Key key){ return !sf::Keyboard::isKeyPressed(key); });
}

void ClassicCat::draw_mouse(sf::RenderWindow& window) {
    // update mouse and paw position
    auto pss2 = update_paw_position(input::get_mouse_input().get_position());

    window.draw(device);

    // draw mouse paw
    draw_paw(window, pss2);
}

} // namespace cats
