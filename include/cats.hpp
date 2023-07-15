// Interface for cats

#pragma once

#include <memory>
#include <SFML/Graphics.hpp>
#include "json/json.h"

namespace cats
{

class ICat
{
public:

    // Initilizes cat
    virtual bool init() = 0;

    // Draws the cat: must not be called before init()
    virtual void draw() = 0;

    // Virtual destructor
    virtual ~ICat() {}
};

class OsuCat : public ICat
{
public:

    bool init() override;
    void draw() override;

private:
    Json::Value left_key_value, right_key_value, smoke_key_value, wave_key_value;
    int offset_x, offset_y;
    int paw_r, paw_g, paw_b, paw_a;
    int paw_edge_r, paw_edge_g, paw_edge_b, paw_edge_a;
    double scale;
    bool is_mouse, is_left_handed, is_enable_toggle_smoke;
    sf::Sprite bg, up, left, right, device, smoke, wave;

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

    bool init() override;
    void draw() override;

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

    bool init() override;
    void draw() override;

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

    bool init() override;
    void draw() override;

private:
    void draw_4K();
    void draw_7K();

    sf::Sprite bg, left_handup, right_handup, left_hand[3], right_hand[3];
    sf::Sprite left_4K[2], right_4K[2], left_7K[4], right_7K[4];
    int left_key_value_4K[2], right_key_value_4K[2];
    int left_key_value_7K[4], right_key_value_7K[4];
    bool is_4K;
};

std::unique_ptr<ICat> get_cat(int mode);

}
