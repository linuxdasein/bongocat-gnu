// Generic interface for input handling

#pragma once

#include <memory>

namespace input
{

class IMouse
{
public:

    // Get the mouse position. Returns a pair of coordinates,
    // forming a point in a square [0,1]x[0,1]
    virtual std::pair<double, double> get_position() = 0;

    // Returns true if the left mouse button is pressed
    virtual bool is_left_button_pressed() = 0;

    // Returns true if the right mouse button is pressed
    virtual bool is_right_button_pressed() = 0;

    virtual ~IMouse() {};
};


std::unique_ptr<IMouse> create_mouse_handler(void* display);

}
