#pragma once
#define BONGO_KEYPRESS_THRESHOLD 0
#define MAX_FRAMERATE 60

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <set>

#include <time.h>
#include <string.h>

#include <cats.hpp>
#include <input.hpp>

namespace data {
const Json::Value& get_cfg();
sf::Vector2i get_cfg_window_size();
sf::Transform get_cfg_window_transform();
std::set<int> json_key_to_scancodes(const Json::Value& key_array);
bool is_intersection(const std::vector<std::set<int>>& sets);

void error_msg(std::string error, std::string title);

void init();

sf::Texture &load_texture(std::string path);
}; // namespace data

namespace input {
bool init(int width, int height);

bool is_pressed(int key_code);

bool is_joystick_connected();
bool is_joystick_pressed(int key_code);

IMouse& get_mouse_input();

void drawDebugPanel(sf::RenderWindow& window);

void cleanup();
}; // namespace input
