#include "header.hpp"

namespace cats {

bool TaikoCat::init(const Json::Value& cfg) {
    // getting configs
    bool chk[256];
    std::fill(chk, chk + 256, false);
    Json::Value taiko = cfg["taiko"];

    rim_key_value[0] = taiko["leftRim"];
    for (Json::Value &v : rim_key_value[0]) {
        chk[v.asInt()] = true;
    }
    centre_key_value[0] = taiko["leftCentre"];
    for (Json::Value &v : centre_key_value[0]) {
        if (chk[v.asInt()]) {
            data::error_msg("Overlapping osu!taiko keybinds", "Error reading configs");
            return false;
        }
    }

    std::fill(chk, chk + 256, false);
    rim_key_value[1] = taiko["rightRim"];
    for (Json::Value &v : rim_key_value[1]) {
        chk[v.asInt()] = true;
    }
    centre_key_value[1] = taiko["rightCentre"];
    for (Json::Value &v : centre_key_value[1]) {
        if (chk[v.asInt()]) {
            data::error_msg("Overlapping osu!taiko keybinds", "Error reading configs");
            return false;
        }
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

void TaikoCat::draw(sf::RenderWindow& window, const sf::RenderStates& rst) {
    window.draw(bg, rst);

    // 0 for left side, 1 for right side
    for (int i = 0; i < 2; i++) {
        bool rim_key = false;
        for (Json::Value &v : rim_key_value[i]) {
            if (input::is_pressed(v.asInt())) {
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
        for (Json::Value &v : centre_key_value[i]) {
            if (input::is_pressed(v.asInt())) {
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
            window.draw(up[i], rst);
        }
        if (key_state[i] == 1) {
            if ((clock() - timer_centre_key[i]) / CLOCKS_PER_SEC > BONGO_KEYPRESS_THRESHOLD) {
                window.draw(rim[i], rst);
                timer_rim_key[i] = clock();
            } else {
                window.draw(up[i], rst);
            }
        } else if (key_state[i] == 2) {
            if ((clock() - timer_rim_key[i]) / CLOCKS_PER_SEC > BONGO_KEYPRESS_THRESHOLD) {
                window.draw(centre[i], rst);
                timer_centre_key[i] = clock();
            } else {
                window.draw(up[i], rst);
            }
        }
    }
}

} // namespace cats
