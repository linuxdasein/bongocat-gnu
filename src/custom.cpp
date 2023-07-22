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

    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
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

    void draw(sf::RenderWindow& window) {
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
            window.draw(default_sprite);
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
                on_key.draw(window);
            } else {
                window.draw(default_sprite);
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

            offset_x = custom["offsetX"].asInt();
            offset_y = custom["offsetY"].asInt();
            scale = custom["scalar"].asDouble();

            paw_r = custom["paw"][0].asInt();
            paw_g = custom["paw"][1].asInt();
            paw_b = custom["paw"][2].asInt();
            paw_a = custom["paw"].size() == 3 ? 255 : custom["paw"][3].asInt();

            paw_edge_r = custom["pawEdge"][0].asInt();
            paw_edge_g = custom["pawEdge"][1].asInt();
            paw_edge_b = custom["pawEdge"][2].asInt();
            paw_edge_a = custom["pawEdge"].size() == 3 ? 255 : custom["pawEdge"][3].asInt();

            if (!custom.isMember("mouseImage") || !custom["mouseImage"].isString()) {
                data::error_msg("Mouse image not found", "Error reading config");
                return false;
            }
            mouse.setTexture(data::load_texture(custom["mouseImage"].asString()));
        }

        // initializing pss and pss2 (kuvster's magic)
        Json::Value paw_draw_info = cfg["mousePaw"];
        x_paw_start = paw_draw_info["pawStartingPoint"][0].asInt();
        y_paw_start = paw_draw_info["pawStartingPoint"][1].asInt();

        x_paw_end = paw_draw_info["pawEndingPoint"][0].asInt();
        y_paw_end = paw_draw_info["pawEndingPoint"][1].asInt();
    } catch (...) {
        return false;
    }
    return true;
}

void CustomCat::draw(sf::RenderWindow& window) {
    window.draw(bg);

    if (is_mouse) {
        auto [fx, fy] = input::get_mouse_input().get_position();

        // apparently, this is a linear transform, intented to move the point to some position,
        // which in general can be specific for each mode. TODO: reduce the amount of arcane number magic in this code.
        double x = -97 * fx + 44 * fy + 184;
        double y = -76 * fx - 40 * fy + 324;

        int oof = 6;
        std::vector<double> pss = {(float) x_paw_start, (float) y_paw_start};
        double dist = hypot(x_paw_start - x, y_paw_start - y);
        double centreleft0 = x_paw_start - 0.7237 * dist / 2;
        double centreleft1 = y_paw_start + 0.69 * dist / 2;
        for (int i = 1; i < oof; i++) {
            std::vector<double> bez = {(float) x_paw_start, (float) y_paw_start, centreleft0, centreleft1, x, y};
            auto [p0, p1] = input::bezier(1.0 * i / oof, bez, 6);
            pss.push_back(p0);
            pss.push_back(p1);
        }
        pss.push_back(x);
        pss.push_back(y);
        double a = y - centreleft1;
        double b = centreleft0 - x;
        double le = hypot(a, b);
        a = x + a / le * 60;
        b = y + b / le * 60;
        dist = hypot(x_paw_end - a, y_paw_end - b);
        double centreright0 = x_paw_end - 0.6 * dist / 2;
        double centreright1 = y_paw_end + 0.8 * dist / 2;
        int push = 20;
        double s = x - centreleft0;
        double t = y - centreleft1;
        le = hypot(s, t);
        s *= push / le;
        t *= push / le;
        double s2 = a - centreright0;
        double t2 = b - centreright1;
        le = hypot(s2, t2);
        s2 *= push / le;
        t2 *= push / le;
        for (int i = 1; i < oof; i++) {
            std::vector<double> bez = {x, y, x + s, y + t, a + s2, b + t2, a, b};
            auto [p0, p1] = input::bezier(1.0 * i / oof, bez, 8);
            pss.push_back(p0);
            pss.push_back(p1);
        }
        pss.push_back(a);
        pss.push_back(b);
        for (int i = oof - 1; i > 0; i--) {
            std::vector<double> bez = {1.0 * x_paw_end, 1.0 * y_paw_end, centreright0, centreright1, a, b};
            auto [p0, p1] = input::bezier(1.0 * i / oof, bez, 6);
            pss.push_back(p0);
            pss.push_back(p1);
        }
        pss.push_back(x_paw_end);
        pss.push_back(y_paw_end);
        double mpos0 = (a + x) / 2 - 52 - 15;
        double mpos1 = (b + y) / 2 - 34 + 5;
        double dx = -38;
        double dy = -50;

        const int iter = 25;

        std::vector<double> pss2 = {pss[0] + dx, pss[1] + dy};
        for (int i = 1; i < iter; i++) {
            auto [p0, p1] = input::bezier(1.0 * i / iter, pss, 38);
            pss2.push_back(p0 + dx);
            pss2.push_back(p1 + dy);
        }
        pss2.push_back(pss[36] + dx);
        pss2.push_back(pss[37] + dy);

        mouse.setPosition(mpos0 + dx + offset_x, mpos1 + dy + offset_y);

        // drawing mouse on top
        if (is_mouse_on_top) {
            window.draw(mouse);
        }

        // drawing arms
        sf::VertexArray fill(sf::TriangleStrip, 26);
        for (int i = 0; i < 26; i += 2) {
            fill[i].position = sf::Vector2f(pss2[i], pss2[i + 1]);
            fill[i + 1].position = sf::Vector2f(pss2[52 - i - 2], pss2[52 - i - 1]);
            fill[i].color = sf::Color(paw_r, paw_g, paw_b, paw_a);
            fill[i + 1].color = sf::Color(paw_r, paw_g, paw_b, paw_a);
        }
        window.draw(fill);

        // drawing first arm arc
        int shad = paw_edge_a / 3;
        sf::VertexArray edge(sf::TriangleStrip, 52);
        double width = 7;
        sf::CircleShape circ(width / 2);
        circ.setFillColor(sf::Color(paw_edge_r, paw_edge_g, paw_edge_b, shad));
        circ.setPosition(pss2[0] - width / 2, pss2[1] - width / 2);
        window.draw(circ);
        for (int i = 0; i < 50; i += 2) {
            double vec0 = pss2[i] - pss2[i + 2];
            double vec1 = pss2[i + 1] - pss2[i + 3];
            double dist = hypot(vec0, vec1);
            edge[i].position = sf::Vector2f(pss2[i] + vec1 / dist * width / 2, pss2[i + 1] - vec0 / dist * width / 2);
            edge[i + 1].position = sf::Vector2f(pss2[i] - vec1 / dist * width / 2, pss2[i + 1] + vec0 / dist * width / 2);
            edge[i].color = sf::Color(paw_edge_r, paw_edge_g, paw_edge_b, shad);
            edge[i + 1].color = sf::Color(paw_edge_r, paw_edge_g, paw_edge_b, shad);
            width -= 0.08;
        }
        double vec0 = pss2[50] - pss2[48];
        double vec1 = pss2[51] - pss2[49];
        dist = hypot(vec0, vec1);
        edge[51].position = sf::Vector2f(pss2[50] + vec1 / dist * width / 2, pss2[51] - vec0 / dist * width / 2);
        edge[50].position = sf::Vector2f(pss2[50] - vec1 / dist * width / 2, pss2[51] + vec0 / dist * width / 2);
        edge[50].color = sf::Color(paw_edge_r, paw_edge_g, paw_edge_b, shad);
        edge[51].color = sf::Color(paw_edge_r, paw_edge_g, paw_edge_b, shad);
        window.draw(edge);
        circ.setRadius(width / 2);
        circ.setPosition(pss2[50] - width / 2, pss2[51] - width / 2);
        window.draw(circ);

        // drawing second arm arc
        sf::VertexArray edge2(sf::TriangleStrip, 52);
        width = 6;
        sf::CircleShape circ2(width / 2);
        circ2.setFillColor(sf::Color(paw_edge_r, paw_edge_g, paw_edge_b, paw_edge_a));
        circ2.setPosition(pss2[0] - width / 2, pss2[1] - width / 2);
        window.draw(circ2);
        for (int i = 0; i < 50; i += 2) {
            vec0 = pss2[i] - pss2[i + 2];
            vec1 = pss2[i + 1] - pss2[i + 3];
            double dist = hypot(vec0, vec1);
            edge2[i].position = sf::Vector2f(pss2[i] + vec1 / dist * width / 2, pss2[i + 1] - vec0 / dist * width / 2);
            edge2[i + 1].position = sf::Vector2f(pss2[i] - vec1 / dist * width / 2, pss2[i + 1] + vec0 / dist * width / 2);
            edge2[i].color = sf::Color(paw_edge_r, paw_edge_g, paw_edge_b, paw_edge_a);
            edge2[i + 1].color = sf::Color(paw_edge_r, paw_edge_g, paw_edge_b, paw_edge_a);
            width -= 0.08;
        }
        vec0 = pss2[50] - pss2[48];
        vec1 = pss2[51] - pss2[49];
        dist = hypot(vec0, vec1);
        edge2[51].position = sf::Vector2f(pss2[50] + vec1 / dist * width / 2, pss2[51] - vec0 / dist * width / 2);
        edge2[50].position = sf::Vector2f(pss2[50] - vec1 / dist * width / 2, pss2[51] + vec0 / dist * width / 2);
        edge2[50].color = sf::Color(paw_edge_r, paw_edge_g, paw_edge_b, paw_edge_a);
        edge2[51].color = sf::Color(paw_edge_r, paw_edge_g, paw_edge_b, paw_edge_a);
        window.draw(edge2);
        circ2.setRadius(width / 2);
        circ2.setPosition(pss2[50] - width / 2, pss2[51] - width / 2);
        window.draw(circ2);
    }

    for (key_container& current : key_containers) {
        current.draw(window);
    }

    // drawing mouse at the bottom
    if (is_mouse && !is_mouse_on_top) {
        window.draw(mouse);
    }
}

} // namespace cats
