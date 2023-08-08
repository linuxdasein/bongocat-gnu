#include <cats.hpp>
#include <input.hpp>
#include <math.h>

namespace cats
{

// bezier curve for osu and custom
std::pair<double, double> bezier(double ratio, std::vector<double> &points, int length) {
    double fact[22] = {0.001, 0.001, 0.002, 0.006, 0.024, 0.12, 0.72, 5.04, 40.32, 362.88, 3628.8, 39916.8, 479001.6, 6227020.8, 87178291.2, 1307674368.0, 20922789888.0, 355687428096.0, 6402373705728.0, 121645100408832.0, 2432902008176640.0, 51090942171709440.0};
    int nn = (length / 2) - 1;
    double xx = 0;
    double yy = 0;

    for (int point = 0; point <= nn; point++) {
        double tmp = fact[nn] / (fact[point] * fact[nn - point]) * pow(ratio, point) * pow(1 - ratio, nn - point);
        xx += points[2 * point] * tmp;
        yy += points[2 * point + 1] * tmp;
    }

    return std::make_pair(xx / 1000, yy / 1000);
}

void MousePaw::set_mouse_parameters(sf::Vector2i of, double sc) {
    offset = of;
    scale = sc;
}

bool MousePaw::init(const Json::Value& mouse_cfg, const Json::Value& paw_draw_info) {
    paw_r = mouse_cfg["paw"][0].asInt();
    paw_g = mouse_cfg["paw"][1].asInt();
    paw_b = mouse_cfg["paw"][2].asInt();
    paw_a = mouse_cfg["paw"].size() == 3 ? 255 : mouse_cfg["paw"][3].asInt();

    paw_edge_r = mouse_cfg["pawEdge"][0].asInt();
    paw_edge_g = mouse_cfg["pawEdge"][1].asInt();
    paw_edge_b = mouse_cfg["pawEdge"][2].asInt();
    paw_edge_a = mouse_cfg["pawEdge"].size() == 3 ? 255 : mouse_cfg["pawEdge"][3].asInt();

    // initializing pss and pss2 (kuvster's magic)
    x_paw_start = paw_draw_info["pawStartingPoint"][0].asInt();
    y_paw_start = paw_draw_info["pawStartingPoint"][1].asInt();

    x_paw_end = paw_draw_info["pawEndingPoint"][0].asInt();
    y_paw_end = paw_draw_info["pawEndingPoint"][1].asInt();

    device.setScale(scale, scale);

    return true;
}

std::vector<double> MousePaw::update_paw_position(std::pair<double, double> mouse_pos) {
    auto [fx, fy] = mouse_pos;
    
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
        auto [p0, p1] = bezier(1.0 * i / oof, bez, 6);
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
        auto [p0, p1] = bezier(1.0 * i / oof, bez, 8);
        pss.push_back(p0);
        pss.push_back(p1);
    }
    pss.push_back(a);
    pss.push_back(b);
    for (int i = oof - 1; i > 0; i--) {
        std::vector<double> bez = {1.0 * x_paw_end, 1.0 * y_paw_end, centreright0, centreright1, a, b};
        auto [p0, p1] = bezier(1.0 * i / oof, bez, 6);
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
        auto [p0, p1] = bezier(1.0 * i / iter, pss, 38);
        pss2.push_back(p0 + dx);
        pss2.push_back(p1 + dy);
    }
    pss2.push_back(pss[36] + dx);
    pss2.push_back(pss[37] + dy);

    device.setPosition(mpos0 + dx + offset.x, mpos1 + dy + offset.y);

    return pss2;
}

void MousePaw::draw_paw(sf::RenderWindow& window, const std::vector<double>& pss2) {
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
    double dist = hypot(vec0, vec1);
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

}