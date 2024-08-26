#include <input.hpp>
#include <logger.hpp>
#include <SFML/Window/Mouse.hpp>
#include <cstring>
#include <cstdlib>

namespace input
{

bool MouseBase::is_left_button_pressed() {
    return sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
}

bool MouseBase::is_right_button_pressed() {
    return sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
}

class MouseSfml : public MouseBase
{
public:

    MouseSfml(bool left_handed);
    ~MouseSfml() = default;

    // get the mouse position
    std::pair<double, double> get_position() override;

private:
    bool is_left_handed;
};

MouseSfml::MouseSfml(bool left_handed) 
    : is_left_handed(left_handed) {}

std::pair<double, double> MouseSfml::get_position() {
    // get global mouse postion in screen coordinates
    sf::Vector2i mouse_pos = sf::Mouse::getPosition();
    auto video_mode = sf::VideoMode::getDesktopMode();

    // project into a unit square
    float x = float(mouse_pos.x) / video_mode.width;
    float y = float(mouse_pos.y) / video_mode.height;

    if (is_left_handed) {
        x = 1.f - x;
    }

    return std::make_pair(x, y);
}

std::unique_ptr<IMouse> create_mouse_handler(bool is_left_handed) {
    const char* xdg_session_type = getenv("XDG_SESSION_TYPE");
    // unfortunately, xdotool does not work on Wayland sessions. The probmlem is that Wayland does not allow to get the mouse position
    // if it is not hovered over the application. That's a security measure preventing applications from watching what a user does outside 
    // of the application scope. currently (as of may 2023) there is no perfect solution to handle such a predicament. However, the
    // mouse cursor is still exposed for another X11 apps running via XWayland (those include Steam and apps running via Proton), 
    // since XWayland is an X11 server itself. Despite the app is somewhat usable with XWayland, wayland sessions are considered unsupported; 
    if(xdg_session_type && !strcmp(xdg_session_type, "wayland")) {
        // some wayland specific implementations may be added here later, but for now
        // use a simple implementation which utilizes only SFML API to discover mouse position
        logger::info("Mouse tracking is not fully supported in Wayland session");
    }
    #ifdef BONGO_ENABLE_XDO
    else {
        // Leave Xorg specific stuff here
        return create_xdo_mouse_handler(is_left_handed);
    }
    #endif

    return std::make_unique<MouseSfml>(is_left_handed);
}

}
