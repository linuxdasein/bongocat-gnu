#include "header.hpp"
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
    KeyActionData{sf::Keyboard::Num1,  "img/standard/keyboard/0.png",  "img/standard/lefthand/0.png"},
    KeyActionData{sf::Keyboard::Num2,  "img/standard/keyboard/1.png",  "img/standard/lefthand/1.png"},
    KeyActionData{sf::Keyboard::Num3,  "img/standard/keyboard/2.png",  "img/standard/lefthand/2.png"},
    KeyActionData{sf::Keyboard::Num4,  "img/standard/keyboard/3.png",  "img/standard/lefthand/3.png"},
    KeyActionData{sf::Keyboard::Num5,  "img/standard/keyboard/4.png",  "img/standard/lefthand/4.png"},
    KeyActionData{sf::Keyboard::Num6,  "img/standard/keyboard/5.png",  "img/standard/lefthand/5.png"},
    KeyActionData{sf::Keyboard::Num7,  "img/standard/keyboard/6.png",  "img/standard/lefthand/6.png"},
    KeyActionData{sf::Keyboard::Q,     "img/standard/keyboard/7.png",  "img/standard/lefthand/7.png"},
    KeyActionData{sf::Keyboard::E,     "img/standard/keyboard/8.png",  "img/standard/lefthand/8.png"},
    KeyActionData{sf::Keyboard::R,     "img/standard/keyboard/9.png",  "img/standard/lefthand/9.png"},
    KeyActionData{sf::Keyboard::Space, "img/standard/keyboard/10.png", "img/standard/lefthand/10.png"},
    KeyActionData{sf::Keyboard::A,     "img/standard/keyboard/11.png", "img/standard/lefthand/11.png"},
    KeyActionData{sf::Keyboard::D,     "img/standard/keyboard/12.png", "img/standard/lefthand/12.png"},
    KeyActionData{sf::Keyboard::S,     "img/standard/keyboard/13.png", "img/standard/lefthand/13.png"},
    KeyActionData{sf::Keyboard::W,     "img/standard/keyboard/14.png", "img/standard/lefthand/14.png"},
};

}

namespace cats {

bool StandardCat::init(const Json::Value& cfg) {

    cat.setTexture(data::load_texture("img/standard/catbg.png"));
    left_paw.setTexture(data::load_texture("img/standard/lefthand/leftup.png"));

    for( const auto& key: actions_data ) {
        key_actions[key.key_code] = std::make_unique<PawAction>(
            sf::Sprite(data::load_texture(key.paw_texture)),
            sf::Sprite(data::load_texture(key.key_texture))
        );
    }

    return true;
}

void StandardCat::draw(sf::RenderWindow& window) {
    window.draw(cat);
    draw_mouse(window);

    for(const auto& key : key_actions) {
        if(sf::Keyboard::isKeyPressed(key.first)) {
            window.draw(*key.second);
            return;
        }
    }

    window.draw(left_paw);
}

void StandardCat::draw_mouse(sf::RenderWindow& window) {
    
}

} // namespace cats
