#include <SFML/System/Vector2.hpp>
#include <header.hpp>
extern "C" {
#include <xdo.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
}

#include <cstring>
#include <string>
#include <set>

namespace input
{

class MouseBase : public IMouse {
public:
    bool is_left_button_pressed() override;
    bool is_right_button_pressed() override;
};

bool MouseBase::is_left_button_pressed() {
    return sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
}

bool MouseBase::is_right_button_pressed() {
    return sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
}

class MouseXdo : public MouseBase
{
public:

    MouseXdo(Display* display, bool left_handed);
    ~MouseXdo();

    // Get the mouse position
    std::pair<double, double> get_position() override;

private:
    void poll_x11_events();
    void process_x11_event(const XEvent& evt);
    std::string print_window_name(Window w);

    xdo_t* xdo;
    Display* dpy;

    bool is_left_handed;
    bool is_mouse_grab_mode = false;
    unsigned int screen_w, screen_h;
    std::set<Window> active_windows;
    Window curent_grabbing_window = 0;
};

MouseXdo::MouseXdo(Display* display, bool left_handed)
    : dpy(display)
    , is_left_handed(left_handed) {
    xdo = xdo_new(NULL);

    // Get the desktop resolution
    int num_sizes;
    Rotation current_rotation;

    Window root = RootWindow(dpy, 0);
    XRRScreenSize *xrrs = XRRSizes(dpy, 0, &num_sizes);

    XRRScreenConfiguration *conf = XRRGetScreenInfo(dpy, root);
    SizeID current_size_id = XRRConfigCurrentConfiguration(conf, &current_rotation);

    screen_w = xrrs[current_size_id].width;
    screen_h = xrrs[current_size_id].height;
}

MouseXdo::~MouseXdo() {
    xdo_free(xdo);
}

std::string MouseXdo::print_window_name(Window w) {
    unsigned char* name_ret;
    int name_len_ret;
    int name_type;

    std::string window_name;

    xdo_get_window_name(xdo, w, &name_ret, &name_len_ret, &name_type);
    if(name_ret!=nullptr)
        window_name = std::string((const char*)name_ret, name_len_ret);
    return window_name;
}

void MouseXdo::poll_x11_events() {
    // Process all pending events
    while(XPending(dpy)) {
        XEvent evt;
        XNextEvent(dpy, &evt);
        process_x11_event(evt);
    }
}

void MouseXdo::process_x11_event(const XEvent& evt) {
    switch(evt.type) {
        case EnterNotify: {
            // The mouse cursor has entered a window
            if(evt.xcrossing.mode == NotifyGrab) {
                // The window has grabbed the mouse pointer
                is_mouse_grab_mode = true;
                curent_grabbing_window = evt.xcrossing.window;
            }
            else if(evt.xcrossing.mode == NotifyUngrab) {
                // Ignore event if ungrabbed cursor enters the grabbing window
                if(evt.xcrossing.window != curent_grabbing_window) {
                    is_mouse_grab_mode = false;
                }
            }
        }
        break;
        case LeaveNotify: {
            // The mouse cursor has left a window
            if(evt.xcrossing.mode == NotifyGrab) {
                // The window has grabbed the mouse pointer
                is_mouse_grab_mode = true;
            }
            else if(evt.xcrossing.mode == NotifyUngrab) {
                // The window has ungrabbed the mouse pointer
                is_mouse_grab_mode = false;
            }
        }
        break;
        case FocusIn: {
            // if the current grabbing window has got focused again,
            // restore mouse grabbing mode
            if(evt.xfocus.window == curent_grabbing_window) {
                is_mouse_grab_mode = true;
            }
        }
        break;
        case FocusOut: {
            // if the current grabbing window has lost focus,
            // disable mouse grabbing mode
            if(evt.xfocus.window == curent_grabbing_window) {
                is_mouse_grab_mode = false;
            }
        }
        break;
        case DestroyNotify: {
            // if the current grabbing window has been closed,
            // disable mouse grabbing mode and delete the window's handle
            // from the set of windows we receive events from
            if(evt.xdestroywindow.window == curent_grabbing_window) {
                is_mouse_grab_mode = false;
            }
            active_windows.erase(evt.xdestroywindow.window);
        }
        break;
    }
}

std::pair<double, double> MouseXdo::get_position() {
    // The point of this code is that we want to track mouse position differently
    // depending on whether the mouse cursor is grabbed by a window or not.
    // If the active window is grabbing the cursor, then we limit the tracking box
    // size to the active windows's dimensions.
    double sbox_pos_x, sbox_pos_y, sbox_width, sbox_height;
    Window foreground_window;

    if (0 == xdo_get_active_window(xdo, &foreground_window)) {
        // Check if we've already subcribed on the window's events
        if (active_windows.find(foreground_window) == active_windows.end()) {
            // Once an active window is found, subscribe to its events we want to receive
            long evt_types = EnterWindowMask | LeaveWindowMask | FocusChangeMask | StructureNotifyMask;
            XSelectInput(dpy, foreground_window, evt_types);
            active_windows.insert(foreground_window);
        }
        poll_x11_events();
    }

    // Initialize with the default values; by default
    // assume that no window is grabbing the mouse cursor
    sbox_width = screen_w;
    sbox_height = screen_h;
    sbox_pos_x = sbox_pos_y = 0;

    if (is_mouse_grab_mode) {
        // The mouse cursor is being grabbed by the active window
        int window_pos_x, window_pos_y;
        if(0 == xdo_get_window_location(xdo, foreground_window, &window_pos_x, &window_pos_y, NULL)) {
            unsigned int width_ret, height_ret;

            if(0 == xdo_get_window_size(xdo, foreground_window, &width_ret, &height_ret)) {
                sbox_width = width_ret;
                sbox_height = height_ret;
                sbox_pos_x = window_pos_x;
                sbox_pos_y = window_pos_y;
            }
        }
    }

    double x = 0, y = 0;
    int px = 0, py = 0;

    if (0 == xdo_get_mouse_location(xdo, &px, &py, NULL)) {
        // transform mouse screen coordinates into coordinates in a unit box
        double fx = (1.0 * px - sbox_pos_x) / sbox_width;

        if (is_left_handed) {
            fx = 1 - fx;
        }

        double fy = (1.0 * py - sbox_pos_y) / sbox_height;

        fx = std::min(fx, 1.0);
        fx = std::max(fx, 0.0);

        fy = std::min(fy, 1.0);
        fy = std::max(fy, 0.0);

        x = fx;
        y = fy;
    }

    return std::make_pair(x, y);
}

class MouseSfml : public MouseBase
{
public:

    MouseSfml(Display* display, bool left_handed);
    ~MouseSfml() = default;

    // get the mouse position
    std::pair<double, double> get_position() override;

private:
    bool is_left_handed;
};

MouseSfml::MouseSfml(Display* display, bool left_handed) 
    : is_left_handed(left_handed) {}

std::pair<double, double> MouseSfml::get_position() {
    // get global mouse postion in screen coordinates
    sf::Vector2i mouse_pos = sf::Mouse::getPosition();
    auto video_mode = sf::VideoMode::getDesktopMode();

    // project into a unit square
    float x = float(mouse_pos.x) / video_mode.size.x;
    float y = float(mouse_pos.y) / video_mode.size.y;

    if (is_left_handed) {
        x = 1.f - x;
    }

    return std::make_pair(x, y);
}

std::unique_ptr<IMouse> create_mouse_handler(void* pdisplay, bool is_left_handed) {
    Display *display = static_cast<Display *>(pdisplay);
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
        return std::make_unique<MouseSfml>(display, is_left_handed);
    }
    else {
        // Leave Xorg specific stuff here
        return std::make_unique<MouseXdo>(display, is_left_handed);
    }
}

}
