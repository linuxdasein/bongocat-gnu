#pragma once
#define BONGO_KEYPRESS_THRESHOLD 0
#define WINDOW_WIDTH 612
#define WINDOW_HEIGHT 352
#define MAX_FRAMERATE 60

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <map>

#include <time.h>
#include <string.h>

#include <cats.hpp>
#include <input.hpp>

namespace data {
const Json::Value& get_cfg();

void error_msg(std::string error, std::string title);

void init();

sf::Texture &load_texture(std::string path);
}; // namespace data

namespace input {
bool init();

bool is_pressed(int key_code);

bool is_joystick_connected();
bool is_joystick_pressed(int key_code);

IMouse& get_mouse_input();

void drawDebugPanel(sf::RenderWindow& window);

void cleanup();
}; // namespace input
