#include "header.hpp"

namespace cats {

bool CtbCat::init(const Json::Value& cfg) {
    // getting configs
    Json::Value ctb = cfg["catch"];

    left_key_binding = data::json_key_to_scancodes(ctb["left"]);
    right_key_binding = data::json_key_to_scancodes(ctb["right"]);

    if(data::is_intersection({left_key_binding, right_key_binding})) {
        logger::get().error("Error reading configs: Overlapping osu!catch keybinds");
        return false;
    }

    dash_key_binding = data::json_key_to_scancodes(ctb["dash"]);

    // importing sprites
    bg.setTexture(data::load_texture("img/catch/bg.png"));
    mid.setTexture(data::load_texture("img/catch/mid.png"));
    left.setTexture(data::load_texture("img/catch/left.png"));
    right.setTexture(data::load_texture("img/catch/right.png"));
    dash.setTexture(data::load_texture("img/catch/dash.png"));
    up.setTexture(data::load_texture("img/catch/up.png"));

    return true;
}

void CtbCat::update() {
    // drawing left-right keypresses
    bool left_key = false;
    for (int v : left_key_binding) {
        if (input::is_pressed(v)) {
            left_key = true;
            break;
        }
    }
    if (left_key) {
        if (!left_key_state) {
            key_state = 1;
            left_key_state = true;
        }
    } else {
        left_key_state = false;
    }

    bool right_key = false;
    for (int v : right_key_binding) {
        if (input::is_pressed(v)) {
            right_key = true;
            break;
        }
    }
    if (right_key) {
        if (!right_key_state) {
            key_state = 2;
            right_key_state = true;
        }
    } else {
        right_key_state = false;
    }

    if (!left_key_state && !right_key_state) {
        key_state = 0;
    }
}

void CtbCat::draw(sf::RenderTarget& target, sf::RenderStates rst) const {
    target.draw(bg, rst);
    
    if (!left_key_state && !right_key_state) {
        target.draw(mid, rst);
    }
    if (key_state == 1) {
        if ((clock() - timer_right_key) / CLOCKS_PER_SEC > BONGO_KEYPRESS_THRESHOLD) {
            target.draw(left, rst);
            timer_left_key = clock();
        } else {
            target.draw(mid, rst);
        }
    } else if (key_state == 2) {
        if ((clock() - timer_left_key) / CLOCKS_PER_SEC > BONGO_KEYPRESS_THRESHOLD) {
            target.draw(right, rst);
            timer_right_key = clock();
        } else {
            target.draw(mid, rst);
        }
    }

    bool is_dash = false;
    for (const int v : dash_key_binding) {
        if (input::is_pressed(v)) {
            target.draw(dash, rst);
            is_dash = true;
            break;
        }
    }
    if (!is_dash) {
        target.draw(up, rst);
    }
}

} // namespace cats
