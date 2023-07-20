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
    mouse.setTexture(data::load_texture("img/osu/mouse.png"), true);
    mouse.setScale(1.0, 1.0f);

    for( const auto& key: actions_data ) {
        key_actions[key.key_code] = std::make_unique<PawAction>(
            sf::Sprite(data::load_texture(key.paw_texture)),
            sf::Sprite(data::load_texture(key.key_texture))
        );
    }

    offset_x = (cfg["decoration"]["offsetX"])[0].asInt();
    offset_y = (cfg["decoration"]["offsetY"])[0].asInt();
    scale = (cfg["decoration"]["scalar"])[0].asDouble();

    // for this mode use separate paw adjustments from the corresponding section
    Json::Value cfg_std = cfg["standard"];
    x_paw_start = cfg_std["pawStartingPoint"][0].asInt();
    y_paw_start = cfg_std["pawStartingPoint"][1].asInt();

    x_paw_end = cfg_std["pawEndingPoint"][0].asInt();
    y_paw_end = cfg_std["pawEndingPoint"][1].asInt();

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

    window.draw(mouse);

    // drawing arms
    int paw_r, paw_g, paw_b, paw_a;
    int paw_edge_r, paw_edge_g, paw_edge_b, paw_edge_a;

    // colors
    paw_r = paw_g = paw_b = 255;
    paw_edge_r = paw_edge_g = paw_edge_b = 0;
    paw_a = paw_edge_a = 255;

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

} // namespace cats
