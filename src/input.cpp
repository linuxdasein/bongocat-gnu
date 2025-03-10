#include "header.hpp"
#include "input.hpp"
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <memory>
#include <sstream>
#include <iomanip>
#include <SFML/Window.hpp>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#define TOTAl_INPUT_TABLE_SIZE 256
#define JOYSTICK_AXIS_DEADZONE 10.0f
#define JOYSTICK_TRIGGER_DEADZONE 3.0f

namespace input {

int last_joystick_keycode = -1;

std::string debugMessage;
std::string debugBitMessage;

sf::RectangleShape debugBackground;
sf::Font debugFont;
std::unique_ptr<sf::Text> debugText;

static std::unique_ptr<IMouse> g_mouse;

IMouse& get_mouse_input() {
    return *g_mouse;
}

enum JoystickInputMapRange {
    MinButton =     0,
    MaxButton =     31,
    LS_Left,
    LS_Right,
    LS_Up,
    LS_Down,
    RS_Left,
    RS_Right,
    RS_Up,
    RS_Down,
    DPad_Left,
    DPad_Right,
    DPad_Up,
    DPad_Down,
    LTrigger,
    RTrigger
};

Display* dpy;

static int _XlibErrorHandler(Display *display, XErrorEvent *event) {
    return true;
}

int INPUT_KEY_TABLE[TOTAl_INPUT_TABLE_SIZE];

bool init(int width, int height, bool is_left_handed) {
    for (int i = 0; i < TOTAl_INPUT_TABLE_SIZE; i++) {
        if (i >= 48 && i <= 57) {           // number
            INPUT_KEY_TABLE[i] = i - 48 + (int)sf::Keyboard::Key::Num0;
        } else if (i >= 65 && i <= 90) {    // english alphabet
            INPUT_KEY_TABLE[i] = i - 65 + (int)sf::Keyboard::Key::A;
        } else if (i >= 96 && i <= 105) {   // numpad
            INPUT_KEY_TABLE[i] = i - 96 + (int)sf::Keyboard::Key::Numpad0;
        } else if (i >= 112 && i <= 126) {  // function
            INPUT_KEY_TABLE[i] = i - 112 + (int)sf::Keyboard::Key::F1;
        } else {
            INPUT_KEY_TABLE[i] = (int)sf::Keyboard::Key::Unknown;
        }
    }

    INPUT_KEY_TABLE[27] = (int)sf::Keyboard::Key::Escape;
    INPUT_KEY_TABLE[17] = (int)sf::Keyboard::Key::LControl;
    INPUT_KEY_TABLE[16] = (int)sf::Keyboard::Key::LShift;
    INPUT_KEY_TABLE[18] = (int)sf::Keyboard::Key::LAlt;
    INPUT_KEY_TABLE[17] = (int)sf::Keyboard::Key::RControl;
    INPUT_KEY_TABLE[16] = (int)sf::Keyboard::Key::RShift;
    INPUT_KEY_TABLE[18] = (int)sf::Keyboard::Key::RAlt;
    INPUT_KEY_TABLE[93] = (int)sf::Keyboard::Key::Menu;
    INPUT_KEY_TABLE[219] = (int)sf::Keyboard::Key::LBracket;
    INPUT_KEY_TABLE[221] = (int)sf::Keyboard::Key::RBracket;
    INPUT_KEY_TABLE[186] = (int)sf::Keyboard::Key::Semicolon;
    INPUT_KEY_TABLE[188] = (int)sf::Keyboard::Key::Comma;
    INPUT_KEY_TABLE[190] = (int)sf::Keyboard::Key::Period;
    INPUT_KEY_TABLE[222] = (int)sf::Keyboard::Key::Apostrophe;
    INPUT_KEY_TABLE[191] = (int)sf::Keyboard::Key::Slash;
    INPUT_KEY_TABLE[220] = (int)sf::Keyboard::Key::Backslash;
    INPUT_KEY_TABLE[192] = (int)sf::Keyboard::Key::Grave;
    INPUT_KEY_TABLE[187] = (int)sf::Keyboard::Key::Equal;
    INPUT_KEY_TABLE[189] = (int)sf::Keyboard::Key::Hyphen;
    INPUT_KEY_TABLE[32] = (int)sf::Keyboard::Key::Space;
    INPUT_KEY_TABLE[13] = (int)sf::Keyboard::Key::Enter;
    INPUT_KEY_TABLE[8] = (int)sf::Keyboard::Key::Backspace;
    INPUT_KEY_TABLE[9] = (int)sf::Keyboard::Key::Tab;
    INPUT_KEY_TABLE[33] = (int)sf::Keyboard::Key::PageUp;
    INPUT_KEY_TABLE[34] = (int)sf::Keyboard::Key::PageDown;
    INPUT_KEY_TABLE[35] = (int)sf::Keyboard::Key::End;
    INPUT_KEY_TABLE[36] = (int)sf::Keyboard::Key::Home;
    INPUT_KEY_TABLE[45] = (int)sf::Keyboard::Key::Insert;
    INPUT_KEY_TABLE[46] = (int)sf::Keyboard::Key::Delete;
    INPUT_KEY_TABLE[107] = (int)sf::Keyboard::Key::Add;
    INPUT_KEY_TABLE[109] = (int)sf::Keyboard::Key::Subtract;
    INPUT_KEY_TABLE[106] = (int)sf::Keyboard::Key::Multiply;
    INPUT_KEY_TABLE[111] = (int)sf::Keyboard::Key::Divide;
    INPUT_KEY_TABLE[37] = (int)sf::Keyboard::Key::Left;
    INPUT_KEY_TABLE[39] = (int)sf::Keyboard::Key::Right;
    INPUT_KEY_TABLE[38] = (int)sf::Keyboard::Key::Up;
    INPUT_KEY_TABLE[40] = (int)sf::Keyboard::Key::Down;
    INPUT_KEY_TABLE[19] = (int)sf::Keyboard::Key::Pause;

    // Set x11 error handler
    XSetErrorHandler(_XlibErrorHandler);

    dpy = XOpenDisplay(NULL);

    // loading font
    debugFont = data::get_debug_font();

    // initialize debug resource
    debugBackground.setSize(sf::Vector2f(width, height));
    debugBackground.setFillColor(sf::Color(0, 0, 0, 128));

    debugText = std::make_unique<sf::Text>(debugFont);
    debugText->setCharacterSize(14);
    debugText->setFillColor(sf::Color::White);
    debugText->setPosition({10.0f, 4.0f});
    debugText->setString(debugMessage);

    g_mouse = create_mouse_handler(dpy, is_left_handed);

    return true;
}

sf::Keyboard::Key ascii_to_key(int key_code) {
    if (key_code < 0 || key_code >= TOTAl_INPUT_TABLE_SIZE) {
        // out of range
        return sf::Keyboard::Key::Unknown;
    } else {
        return (sf::Keyboard::Key)(INPUT_KEY_TABLE[key_code]);
    }
}

// for some special cases of num dot and such
bool is_pressed_fallback(int key_code) {
    // code snippet from SFML
    KeyCode keycode = XKeysymToKeycode(dpy, key_code);
    if (keycode != 0) {
        char keys[32];
        XQueryKeymap(dpy, keys);
        return (keys[keycode / 8] & (1 << (keycode % 8))) != 0;
    }
    else {
        return false;
    }
}

bool is_pressed(int key_code) {
    if (key_code == 16) {
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)
            || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);
    } else if (key_code == 17) {
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)
            || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl);
    } else {
        sf::Keyboard::Key selected = ascii_to_key(key_code);
        if (selected != sf::Keyboard::Key::Unknown) {
            return sf::Keyboard::isKeyPressed(selected);
        } else {
            return is_pressed_fallback(key_code);
        }
    }
}

bool is_joystick_connected() {
    return sf::Joystick::isConnected(0);
}

bool is_joystick_pressed(int key_code) {
    int id = 0;
    last_joystick_keycode = key_code;

    // joystick button, range 0 - 31
    if (key_code >= MinButton && key_code <= MaxButton) {
        return sf::Joystick::isButtonPressed(id, key_code);
    }
    // joystick axis, range 32 - 45
    else {
        float axis = 0.0f;

        switch (key_code) {
            case LS_Left:
                axis = sf::Joystick::getAxisPosition(id, sf::Joystick::Axis::X);
                return axis <= -JOYSTICK_AXIS_DEADZONE;
            break;

            case LS_Right:
                axis = sf::Joystick::getAxisPosition(id, sf::Joystick::Axis::X);
                return axis >= JOYSTICK_AXIS_DEADZONE;
            break;

            case LS_Up:
                axis = sf::Joystick::getAxisPosition(id, sf::Joystick::Axis::Y);
                return axis <= -JOYSTICK_AXIS_DEADZONE;
            break;

            case LS_Down:
                axis = sf::Joystick::getAxisPosition(id, sf::Joystick::Axis::Y);
                return axis >= JOYSTICK_AXIS_DEADZONE;
            break;

            case RS_Left:
                axis = sf::Joystick::getAxisPosition(id, sf::Joystick::Axis::U);
                return axis <= -JOYSTICK_AXIS_DEADZONE;
            break;

            case RS_Right:
                axis = sf::Joystick::getAxisPosition(id, sf::Joystick::Axis::U);
                return axis >= JOYSTICK_AXIS_DEADZONE;
            break;

            case RS_Up:
                axis = sf::Joystick::getAxisPosition(id, sf::Joystick::Axis::V);
                return axis <= -JOYSTICK_AXIS_DEADZONE;
            break;

            case RS_Down:
                axis = sf::Joystick::getAxisPosition(id, sf::Joystick::Axis::V);
                return axis >= JOYSTICK_AXIS_DEADZONE;
            break;

            case DPad_Left:
                axis = sf::Joystick::getAxisPosition(id, sf::Joystick::Axis::PovX);
                return axis <= -JOYSTICK_AXIS_DEADZONE;
            break;

            case DPad_Right:
                axis = sf::Joystick::getAxisPosition(id, sf::Joystick::Axis::PovX);
                return axis >= JOYSTICK_AXIS_DEADZONE;
            break;

            case DPad_Up:
                axis = sf::Joystick::getAxisPosition(id, sf::Joystick::Axis::PovY);
                return axis <= -JOYSTICK_AXIS_DEADZONE;
            break;

            case DPad_Down:
                axis = sf::Joystick::getAxisPosition(id, sf::Joystick::Axis::PovY);
                return axis >= JOYSTICK_AXIS_DEADZONE;
            break;

            case LTrigger:
                axis = sf::Joystick::getAxisPosition(id, sf::Joystick::Axis::Z);
                return axis >= JOYSTICK_TRIGGER_DEADZONE;
            break;

            case RTrigger:
                axis = sf::Joystick::getAxisPosition(id, sf::Joystick::Axis::R);
                return axis >= JOYSTICK_TRIGGER_DEADZONE;
            break;

            default:
                return false;
            break;
        }
    }

    return false;
}

void drawDebugPanel(sf::RenderWindow& window) {
    if (!is_joystick_connected()) {
        debugText->setString("No joystick found...");
        window.draw(debugBackground);
        window.draw(*debugText);
        return;
    }

    int joy_id = 0;

    std::stringstream result;
    sf::Joystick::Identification info = sf::Joystick::getIdentification(joy_id);

    result << "Joystick connected : " << info.name.toAnsiString() << std::endl;
    result << "Support button : " << sf::Joystick::getButtonCount(joy_id) << std::endl;

    int offset = 0;
    int counter = 0;
    int max_button = (int)sf::Joystick::ButtonCount;
    int max_row = 11;

    for (int i = 0; i < max_button; ++i) {
        int buttonID = (counter + offset);

        bool isPressed = sf::Joystick::isButtonPressed(joy_id, buttonID);
        std::string state = isPressed ? "PRESS" : "release";

        result << std::setw(10) << "Button#" << std::setw(2) << buttonID << std::setw(1) << ": " << std::setw(5) << state;

        counter += max_row;
        bool shouldPrintNextLine = ((i + 1) % 3) == 0;

        if (shouldPrintNextLine) {
            result << std::endl;
            counter = 0;
            offset += 1;
        }
    }

    result << std::endl;
    result << "Axis : " << std::endl;

    sf::Vector2f leftstick_axis;
    sf::Vector2f rightstick_axis;
    sf::Vector2f dpad_axis;
    float left_trigger_axis;
    float right_trigger_axis;

    leftstick_axis.x = sf::Joystick::getAxisPosition(joy_id, sf::Joystick::Axis::X);
    leftstick_axis.y = sf::Joystick::getAxisPosition(joy_id, sf::Joystick::Axis::Y);
    rightstick_axis.x = sf::Joystick::getAxisPosition(joy_id, sf::Joystick::Axis::U);
    rightstick_axis.y = sf::Joystick::getAxisPosition(joy_id, sf::Joystick::Axis::V);
    
    dpad_axis.x = sf::Joystick::getAxisPosition(joy_id, sf::Joystick::Axis::PovX);
    dpad_axis.y = sf::Joystick::getAxisPosition(joy_id, sf::Joystick::Axis::PovY);

    left_trigger_axis = sf::Joystick::getAxisPosition(joy_id, sf::Joystick::Axis::Z); 
    right_trigger_axis = sf::Joystick::getAxisPosition(joy_id, sf::Joystick::Axis::R); 

    result << "LStick : " << "( " << leftstick_axis.x << "," << leftstick_axis.y << " )" << std::endl;
    result << "RStick : " << "( " << rightstick_axis.x << "," << rightstick_axis.y << " )" << std::endl;
    result << "LTrigger : " << left_trigger_axis << std::endl;
    result << "RTrigger : " << right_trigger_axis << std::endl;
    result << "DPad : " << "( " << dpad_axis.x << "," << dpad_axis.y << " )" << std::endl;

    debugText->setString(result.str());

    window.draw(debugBackground);
    window.draw(*debugText);
}

void cleanup() {
    XCloseDisplay(dpy);
}

};

