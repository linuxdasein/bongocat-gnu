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
sf::Vector2i get_cfg_window_default_size();
sf::Vector2i get_cfg_window_size(const Json::Value &cfg);
sf::Transform get_cfg_window_transform(const Json::Value &cfg);
std::set<int> json_key_to_scancodes(const Json::Value& key_array);
bool is_intersection(const std::vector<std::set<int>>& sets);

bool init();
bool reload_config();
const Json::Value& get_cfg();
sf::Texture &load_texture(std::string path);
sf::Font &get_debug_font();
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

namespace logger {

enum class Severity 
{
    critical,
    medium,
    info,
    debug
};

class ILogger 
{
public:

    // Log a message with a certain severity level
    virtual void log(std::string message, Severity level) = 0;

    // Log a critical error
    void error(std::string message) {
        return log(message, Severity::critical);
    }

    virtual ~ILogger() {};
};

ILogger& get();

} // namespace logger
