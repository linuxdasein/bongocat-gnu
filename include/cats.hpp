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

std::unique_ptr<ICat> get_cat(int mode);

}
