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

extern const sf::Vector2i g_window_default_size;

std::set<int> json_key_to_scancodes(const Json::Value& key_array);
bool is_intersection(const std::vector<std::set<int>>& sets);

bool init();
sf::Texture &load_texture(std::string path);
sf::Font &get_debug_font();
}; // namespace data

namespace input {
bool init(int width, int height, bool is_left_handed = false);

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
    warning,
    info,
    debug
};

class ILogger 
{
public:

    // Log a message with a certain severity level
    virtual void log(std::string message, Severity level) = 0;

    // virtual destructor
    virtual ~ILogger() {};
};

// get global logger instance
ILogger& get();

// Log a critical error
inline void error(std::string message) {
    get().log(message, Severity::critical);
}

// Log a warning message
inline void warn(std::string message) {
    get().log(message, Severity::warning);
}

// Log an information message
inline void info(std::string message) {
    get().log(message, Severity::info);
}

// Log a debug message
inline void debug(std::string message) {
    get().log(message, Severity::debug);
}

} // namespace logger
