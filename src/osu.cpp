#include "header.hpp"

namespace cats {

bool OsuCat::init(const Json::Value& cfg) {
    // getting configs
    Json::Value osu = cfg["osu"];

    is_mouse = osu["mouse"].asBool();
    is_enable_toggle_smoke = osu["toggleSmoke"].asBool();

    left_key_binding = data::json_key_to_scancodes(osu["key1"]);
    right_key_binding = data::json_key_to_scancodes(osu["key2"]);
    wave_key_binding = data::json_key_to_scancodes(osu["wave"]);
    smoke_key_binding = data::json_key_to_scancodes(osu["smoke"]);

    if(data::is_intersection({left_key_binding, right_key_binding, wave_key_binding}))
        data::error_msg("Overlapping osu! keybinds", "Error reading configs");

    is_left_handed = cfg["decoration"]["leftHanded"].asBool();

    // importing sprites
    up.setTexture(data::load_texture("img/osu/up.png"));
    left.setTexture(data::load_texture("img/osu/left.png"));
    right.setTexture(data::load_texture("img/osu/right.png"));
    wave.setTexture(data::load_texture("img/osu/wave.png"));
    if (is_mouse) {
        bg.setTexture(data::load_texture("img/osu/mousebg.png"));
        device.setTexture(data::load_texture("img/osu/mouse.png"), true);
    } else {
        bg.setTexture(data::load_texture("img/osu/tabletbg.png"));
        device.setTexture(data::load_texture("img/osu/tablet.png"), true);
    }
    smoke.setTexture(data::load_texture("img/osu/smoke.png"));

    sf::Vector2i offset;
    double scale;

    if (is_mouse) {
        offset.x = (cfg["decoration"]["offsetX"])[0].asInt();
        offset.y = (cfg["decoration"]["offsetY"])[0].asInt();
        scale = (cfg["decoration"]["scalar"])[0].asDouble();
    } else {
        offset.x = (cfg["decoration"]["offsetX"])[1].asInt();
        offset.y = (cfg["decoration"]["offsetY"])[1].asInt();
        scale = (cfg["decoration"]["scalar"])[1].asDouble();
    }

    // initialize thew mouse paw
    MousePaw::set_mouse_parameters(offset, scale);
    MousePaw::init(cfg["osu"], cfg["mousePaw"]);

    return true;
}

void OsuCat::update() {
    // update mouse and paw position
    update_paw_position(input::get_mouse_input().get_position());

    bool left_key = false;

    for (const int v : left_key_binding) {
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

    for (const int v : right_key_binding) {
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
    
    bool wave_key = false;

    for (const int v : wave_key_binding) {
        if (input::is_pressed(v)) {
            wave_key = true;
            break;
        }
    }

    if (wave_key) {
        if (!wave_key_state) {
            key_state = 3;
            wave_key_state = true;
        }
    } else {
        wave_key_state = false;
    }

    if (!left_key_state && !right_key_state && !wave_key_state) {
        key_state = 0;
    }

    bool is_smoke_key_pressed = false;

    for (const int v : smoke_key_binding) {
        if (input::is_pressed(v)) {
            is_smoke_key_pressed = true;
            break;
        }
    }

    if (is_enable_toggle_smoke) {
        previous_smoke_key_state = current_smoke_key_state;
        current_smoke_key_state = is_smoke_key_pressed;

        bool is_smoke_key_down = current_smoke_key_state && (current_smoke_key_state != previous_smoke_key_state);

        if (is_smoke_key_down) {
            is_toggle_smoke = !is_toggle_smoke;
        }
    }
    else {
        is_toggle_smoke = is_smoke_key_pressed;
    }
}

void OsuCat::draw(sf::RenderTarget& target, sf::RenderStates rst) const {
    target.draw(bg, rst);

    // drawing mouse
    if (is_mouse) {
        target.draw(device, rst);
    }

    // draw mouse paw
    draw_paw(target, rst);

    // drawing keypresses
    if (!left_key_state && !right_key_state && !wave_key_state) {
        target.draw(up, rst);
    }

    if (key_state == 1) {
        if ((clock() - std::max(timer_right_key, timer_wave_key)) / CLOCKS_PER_SEC > BONGO_KEYPRESS_THRESHOLD) {
            if (!is_left_handed) {
                target.draw(left, rst);
            } else {
                target.draw(right, rst);
            }
            timer_left_key = clock();
        } else {
            target.draw(up, rst);
        }
    } else if (key_state == 2) {
        if ((clock() - std::max(timer_left_key, timer_wave_key)) / CLOCKS_PER_SEC > BONGO_KEYPRESS_THRESHOLD) {
            if (!is_left_handed) {
                target.draw(right, rst);
            } else {
                target.draw(left, rst);
            }
            timer_right_key = clock();
        } else {
            target.draw(up, rst);
        }
    } else if (key_state == 3) {
        if ((clock() - std::max(timer_left_key, timer_right_key)) / CLOCKS_PER_SEC > BONGO_KEYPRESS_THRESHOLD) {
            target.draw(wave, rst);
            timer_wave_key = clock();
        } else {
            target.draw(up, rst);
        }
    }

    // drawing tablet
    if (!is_mouse) {
        target.draw(device, rst);
    }
    
    // draw smoke
    if (is_toggle_smoke) {
        target.draw(smoke, rst);
    }
}

} // namespace cats
