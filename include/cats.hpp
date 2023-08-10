// Interface for cats

#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <memory>
#include <SFML/Graphics.hpp>
#include <json/json.h>

#include <list>

namespace cats
{

class ICat
{
public:

    // Initilizes cat
    virtual bool init(const Json::Value& cfg) = 0;

    // Draws the cat: must not be called before init()
    virtual void draw(sf::RenderWindow& window) = 0;

    // Virtual destructor
    virtual ~ICat() {}
};

class MousePaw 
{
protected:

    // Initialize mouse paw with json config
    bool init(const Json::Value& mouse_cfg, const Json::Value& paw_cfg);

    // Update deviice and paw position according to the mouse_pos
    std::vector<sf::Vector2f> update_paw_position(std::pair<double, double> mouse_pos);

    // Display paw represented by its coordinates pss2
    void draw_paw(sf::RenderWindow& window, const std::vector<sf::Vector2f>& pss2);

    // Set offset and scale for mouse sprite
    void set_mouse_parameters(sf::Vector2i offset, double scale);

    sf::Sprite device;
private:
    // draw an arc about an array of points
    void draw_arc(sf::RenderWindow& window, const std::vector<sf::Vector2f>& pss2, sf::Color color, float width);

    double scale = 1.0;
    sf::Vector2i offset = {0, 0};

    int x_paw_start, y_paw_start;
    int x_paw_end, y_paw_end;

    sf::Color paw_color;
    sf::Color paw_edge_color;
};

class OsuCat : public ICat, private MousePaw
{
public:

    bool init(const Json::Value& cfg) override;
    void draw(sf::RenderWindow& window) override;

private:

    Json::Value left_key_value, right_key_value, smoke_key_value, wave_key_value;
    bool is_mouse, is_left_handed, is_enable_toggle_smoke;
    sf::Sprite bg, up, left, right, smoke, wave;

    int key_state = 0;

    bool left_key_state = false;
    bool right_key_state = false;
    bool wave_key_state = false;
    bool previous_smoke_key_state = false;
    bool current_smoke_key_state = false;
    bool is_toggle_smoke = false;
    double timer_left_key = -1;
    double timer_right_key = -1;
    double timer_wave_key = -1;
};

class TaikoCat : public ICat
{
public:

    bool init(const Json::Value& cfg) override;
    void draw(sf::RenderWindow& window) override;

private:
    Json::Value rim_key_value[2], centre_key_value[2];
    sf::Sprite bg, up[2], rim[2], centre[2];
    int key_state[2] = {0, 0};
    bool rim_key_state[2] = {false, false};
    bool centre_key_state[2] = {false, false};
    double timer_rim_key[2] = {-1, -1};
    double timer_centre_key[2] = {-1, -1};
};

class CtbCat : public ICat
{
public:

    bool init(const Json::Value& cfg) override;
    void draw(sf::RenderWindow& window) override;

private:
    Json::Value left_key_value, right_key_value, dash_key_value;
    sf::Sprite bg, mid, left, right, dash, up;

    int key_state = 0;
    bool left_key_state = false;
    bool right_key_state = false;
    double timer_left_key = -1;
    double timer_right_key = -1;
};

class ManiaCat : public ICat
{
public:

    bool init(const Json::Value& cfg) override;
    void draw(sf::RenderWindow& window) override;

private:
    void draw_4K(sf::RenderWindow& window);
    void draw_7K(sf::RenderWindow& window);

    sf::Sprite bg, left_handup, right_handup, left_hand[3], right_hand[3];
    sf::Sprite left_4K[2], right_4K[2], left_7K[4], right_7K[4];
    int left_key_value_4K[2], right_key_value_4K[2];
    int left_key_value_7K[4], right_key_value_7K[4];
    bool is_4K;
};

class CustomCat : public ICat, private MousePaw
{
public:

    bool init(const Json::Value& cfg) override;
    void draw(sf::RenderWindow& window) override;

private:
    sf::Sprite bg;

    bool is_mouse, is_mouse_on_top;
};

class ClassicCat : public ICat, private MousePaw
{
public:

    bool init(const Json::Value& cfg) override;
    void draw(sf::RenderWindow& window) override;

private:
    void draw_mouse(sf::RenderWindow& window);

    sf::Sprite cat, left_paw, mouse;
    std::map<sf::Keyboard::Key, std::unique_ptr<sf::Drawable> > key_actions;

    std::list<sf::Keyboard::Key> keys;
    std::list<sf::Keyboard::Key> pressed_keys;
};

std::unique_ptr<ICat> get_cat(int mode);

}
