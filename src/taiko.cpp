#include "header.hpp"

namespace cats {

bool TaikoCat::init(const Json::Value& cfg) {
    // getting configs
    Json::Value taiko = cfg["taiko"];

    rim_key_binding[0] = data::json_key_to_scancodes(taiko["leftRim"]);
    centre_key_binding[0] = data::json_key_to_scancodes(taiko["leftCentre"]);

    if(data::is_intersection({rim_key_binding[0], centre_key_binding[0]})) {
        logger::get().error("Error reading configs: Overlapping osu!taiko keybinds");
        return false;
    }

    rim_key_binding[1] = data::json_key_to_scancodes(taiko["rightRim"]);
    centre_key_binding[1] = data::json_key_to_scancodes(taiko["rightCentre"]);
    
    if(data::is_intersection({rim_key_binding[1], centre_key_binding[1]})) {
        logger::get().error("Error reading configs: Overlapping osu!taiko keybinds");
        return false;
    }

    // importing sprites
    bg.setTexture(data::load_texture("img/taiko/bg.png"));
    up[0].setTexture(data::load_texture("img/taiko/leftup.png"));
    rim[0].setTexture(data::load_texture("img/taiko/leftrim.png"));
    centre[0].setTexture(data::load_texture("img/taiko/leftcentre.png"));
    up[1].setTexture(data::load_texture("img/taiko/rightup.png"));
    rim[1].setTexture(data::load_texture("img/taiko/rightrim.png"));
    centre[1].setTexture(data::load_texture("img/taiko/rightcentre.png"));

    return true;
}

void TaikoCat::update() {
    // 0 for left side, 1 for right side
    for (int i = 0; i < 2; i++) {
        bool rim_key = false;
        for (const int v : rim_key_binding[i]) {
            if (input::is_pressed(v)) {
                rim_key = true;
                break;
            }
        }
        if (rim_key) {
            if (!rim_key_state[i]) {
                key_state[i] = 1;
                rim_key_state[i] = true;
            }
        } else {
            rim_key_state[i] = false;
        }

        bool centre_key = false;
        for (const int v : centre_key_binding[i]) {
            if (input::is_pressed(v)) {
                centre_key = true;
                break;
            }
        }
        if (centre_key) {
            if (!centre_key_state[i]) {
                key_state[i] = 2;
                centre_key_state[i] = true;
            }
        } else {
            centre_key_state[i] = false;
        }

        if (!rim_key_state[i] && !centre_key_state[i]) {
            key_state[i] = 0;
        }
    }
}

void TaikoCat::draw(sf::RenderTarget& target, sf::RenderStates rst) const {
    target.draw(bg, rst);

    // 0 for left side, 1 for right side
    for (int i = 0; i < 2; i++) {
        if (!rim_key_state[i] && !centre_key_state[i]) {
            target.draw(up[i], rst);
        }
        if (key_state[i] == 1) {
            if ((clock() - timer_centre_key[i]) / CLOCKS_PER_SEC > BONGO_KEYPRESS_THRESHOLD) {
                target.draw(rim[i], rst);
                timer_rim_key[i] = clock();
            } else {
                target.draw(up[i], rst);
            }
        } else if (key_state[i] == 2) {
            if ((clock() - timer_rim_key[i]) / CLOCKS_PER_SEC > BONGO_KEYPRESS_THRESHOLD) {
                target.draw(centre[i], rst);
                timer_centre_key[i] = clock();
            } else {
                target.draw(up[i], rst);
            }
        }
    }
}

} // namespace cats
