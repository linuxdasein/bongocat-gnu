#include "header.hpp"
#include <SFML/Graphics/Transform.hpp>
#include <SFML/System/Vector2.hpp>
#include <cats.hpp>
#include <input.hpp>
#include <math.hpp>
#include <cmath>

namespace cats
{

void MousePaw::set_mouse_parameters(sf::Vector2i of, double sc) {
    offset = of;
    scale = sc;
}

bool MousePaw::init(const Json::Value& mouse_cfg, const Json::Value& paw_draw_info) {
    paw_color.r = mouse_cfg["paw"][0].asInt();
    paw_color.g = mouse_cfg["paw"][1].asInt();
    paw_color.b = mouse_cfg["paw"][2].asInt();
    paw_color.a = mouse_cfg["paw"].size() == 3 ? 255 : mouse_cfg["paw"][3].asInt();

    paw_edge_color.r = mouse_cfg["pawEdge"][0].asInt();
    paw_edge_color.g = mouse_cfg["pawEdge"][1].asInt();
    paw_edge_color.b = mouse_cfg["pawEdge"][2].asInt();
    paw_edge_color.a = mouse_cfg["pawEdge"].size() == 3 ? 255 : mouse_cfg["pawEdge"][3].asInt();

    x_paw_start = paw_draw_info["pawStartingPoint"][0].asInt();
    y_paw_start = paw_draw_info["pawStartingPoint"][1].asInt();

    x_paw_end = paw_draw_info["pawEndingPoint"][0].asInt();
    y_paw_end = paw_draw_info["pawEndingPoint"][1].asInt();

    auto paw_boundary_config = paw_draw_info["pawBoundaryPoints"];

    auto get_point_from_json = [&paw_boundary_config](sf::Vector2i &pt, const std::string& id) {
        auto jp = paw_boundary_config[id];
        if (!jp.isArray() || jp.size() != 2) {
            logger::error("Invalid value in pawBoundaryPoints:" 
                + id + ". An array of two ints is expected");
            return false;
        }

        if (!jp[0].isInt()) {
            logger::error("Invalid value in pawBoundaryPoints:" 
                + id + "[0]. An integer value is expected");
            return false;
        }

        if (!jp[1].isInt()) {
            logger::error("Invalid value in pawBoundaryPoints:" 
                + id + "[1]. An integer value is expected");
            return false;
        }

        pt = { jp[0].asInt(), jp[1].asInt() };

        return true;
    };

    if( !paw_boundary_config.isNull() ) {
        for (auto const& id : paw_boundary_config.getMemberNames()) {
            if("A" == id) {
                if(!get_point_from_json(A, id)) 
                    return false;
            }
            else if ("B" == id) {
                if(!get_point_from_json(B, id)) 
                    return false;
            }
            else if ("C" == id) {
                if(!get_point_from_json(C, id)) 
                    return false;
            }
            else {
                logger::warn("Unexpected key encountered in pawBoundaryPoints section:"
                    + id + ", ignoring it");
            }
        }
    }
    else {
        logger::info("No pawBoundaryPoints section found in config file, using default values");
        A = {146, 274};
        B = {49, 198};
        C = {190, 234};
    }

    device.setScale(scale, scale);
    left_button.setScale(scale, scale);
    right_button.setScale(scale, scale);

    return true;
}

void MousePaw::update_paw_position(std::pair<double, double> mouse_pos) {
    auto [fx, fy] = mouse_pos;

    // project the position from the unit square to a parallelogram domain
    // in the screen coordinates defined by three corner points A, B, C
    const math::point2d m = {
        (B.x - A.x) * fx + (C.x - A.x) * fy + A.x, 
        (B.y - A.y) * fx + (C.y - A.y) * fy + A.y
    };

    const int oof = 6;
    math::BCurve B2(2);
    math::BCurve B3(3);

    // fixed point where the paw arc starts
    const math::point2d paw_start = {(double) x_paw_start, (double) y_paw_start};
    // fixed point where the paw arc ends
    const math::point2d paw_end = {(double)x_paw_end, (double)y_paw_end};

    std::vector<math::point2d> pss = {paw_start};

    double dist = hypot(paw_start.x - m.x, paw_start.y - m.y);
    // sort of unit tangent vector at the paw_start point of the arc
    const math::point2d tangentleft = {-0.7237, 0.69};
    // central control point of the left bezier arc
    const math::point2d centreleft = paw_start + tangentleft * (dist / 2);
    
    // the first arc segment
    std::vector<math::point2d> bez1 = {paw_start, centreleft, m};
    B2.set_control_points(bez1);

    for (int i = 1; i < oof; i++) {
        pss.push_back(B2(1.0 * i / oof));
    }

    pss.push_back(m);

    // rotate centreleft by 90 degrees clockwise
    math::point2d ab;
    ab.x = m.y - centreleft.y;
    ab.y = centreleft.x - m.x;
    // calculate some middle point between mpos and ab
    double le = hypot(ab.x, ab.y);
    ab = m + ab / le * 60.0;
    
    dist = hypot(paw_end.x - ab.x, paw_end.y - ab.y);
    // unit tangent vector at the paw_end point of the arc
    const math::point2d tangentright = {-0.6, 0.8};
    // central control point of the right bezier arc
    const math::point2d centreright = paw_end + tangentright * (dist / 2);

    // calculate "push" vectors, which are intended to move
    // mpos and ab outside of the paw so they form the next 
    // arc segment's control points
    const int push = 20;
    // push vector for mpos
    math::point2d st = m - centreleft;
    le = hypot(st.x, st.y);
    st *= push / le;
    // push vector for ab
    math::point2d st2 = ab - centreright;
    le = hypot(st2.x, st2.y);
    st2 *= push / le;

    // the second segment
    std::vector<math::point2d> bez2 = { m, m + st, ab + st2, ab};
    B3.set_control_points(bez2);
    for (int i = 1; i < oof; i++) {
        pss.push_back(B3(1.0 * i / oof));
    }
    pss.push_back(ab);

    // the third segment
    std::vector<math::point2d> bez3 = { paw_end, centreright, ab};
    B2.set_control_points(bez3);
    for (int i = oof - 1; i > 0; i--) {
        pss.push_back(B2(1.0 * i / oof));
    }
    pss.push_back(paw_end);

    const int iter = 25;
    math::BCurve B18(3 * oof);
    // constructing a high order curve over a set of control points from existing curves
    // is pretty much messed up way to do things... TODO: use a spline instead
    B18.set_control_points(pss);

    std::vector<math::point2d> pss2d = {pss[0]};
    for (int i = 1; i < iter; i++) {
        auto p = B18(1.0 * i / iter);
        pss2d.push_back(p);
    }
    pss2d.push_back(pss[3 * oof]);

    // mouse position is the corner of the mouse sprite
    // need some offset depending on the actual sprite being used
    const math::point2d moffset = {-52 - 15, -34 + 5};
    const math::point2d dpos = (ab + m) / 2.0 + moffset;
    device.setPosition(dpos.x + offset.x, dpos.y + offset.y);
    left_button.setPosition(dpos.x + offset.x, dpos.y + offset.y);
    right_button.setPosition(dpos.x + offset.x, dpos.y + offset.y);

    // convert to float (consider to perform math in float in the first place)
    std::vector<sf::Vector2f> pss2f;
    for(auto pd : pss2d) {
        pss2f.push_back(sf::Vector2f(pd.x, pd.y));
    }

    pss2 = std::move(pss2f);
}

void MousePaw::draw_paw(sf::RenderTarget& target, sf::RenderStates rst) const {
    // drawing arm's body
    const size_t nump = pss2.size();
    sf::VertexArray fill(sf::TriangleStrip, nump);
    for (size_t i = 0; i < nump; i += 2) {
        fill[i].position = pss2[i];
        fill[i + 1].position = pss2[nump - i - 1];
        fill[i].color = fill[i + 1].color = paw_color;
    }
    target.draw(fill, rst);

    // drawing the shadow of the arm arc
    auto paw_edge_color_shad = paw_edge_color;
    paw_edge_color_shad.a /= 3;
    draw_arc(target, rst, paw_edge_color_shad, 7);

    // drawing the line of the arm arc
    draw_arc(target, rst, paw_edge_color, 6);
}

void MousePaw::draw_arc(sf::RenderTarget& target, sf::RenderStates rst, sf::Color color, float width) const {
    // at the first point of the arc we draw a circle shape
    // in order to make arc's beginning rounded
    sf::CircleShape circ(width / 2);
    circ.setFillColor(color);
    // the coordinates of the sprite are the coordinates of the upper left corner
    // of the "enclosing box", so it's needed to set the coordinates accordingly
    sf::Vector2f offset(width / 2, width / 2);
    circ.setPosition(pss2[0] - offset);
    target.draw(circ, rst);

    sf::Transform rotate_left, rotate_right;
    rotate_left.rotate(-90); // counter-clockwise rotation
    rotate_right.rotate(90); // clockwise rotation

    // Next, we draw the arm's arc, using triangle strip
    const size_t nump = pss2.size();
    sf::VertexArray edge(sf::TriangleStrip, nump * 2);
    for (size_t i = 0; i < nump; i += 1, width -= 0.08) {
        // construct a vector, pointing at the direction of the next paw point
        auto vec = (i == nump-1) 
            ? pss2[i-1] - pss2[i]  // the last point edge case; use the 
            : pss2[i] - pss2[i+1]; // previous point to construct the vector
        // set the length of the vector to width/2
        float dist = hypot(vec.x, vec.y);
        vec = vec / dist * (width / 2);

        // to form the triangle strip we use points from the left 
        // and from the right of the current point
        edge[2*i].position   = pss2[i] + rotate_left * vec;
        edge[2*i+1].position = pss2[i] + rotate_right * vec;

        // set the trianle's edge color
        edge[2*i].color = edge[2*i+1].color = color;
    }

    // at the end of the arc also draw a circle shape
    target.draw(edge, rst);
    circ.setRadius(width / 2);
    offset = sf::Vector2f(width / 2, width / 2);
    circ.setPosition(pss2[nump-1] - offset);
    target.draw(circ, rst);
}

}
