#include <header.hpp>
extern "C" {
#include <xdo.h>
}

#include <string>
#include <cmath>

namespace input
{

class MouseXdo : public IMouse
{
public:

    MouseXdo(int h, int v);
    ~MouseXdo();

    // Get the mouse position
    std::pair<double, double> get_position() override;

private:
    xdo_t* xdo;
    int horizontal, vertical;
    int osu_x, osu_y, osu_h, osu_v;
    bool is_letterbox, is_left_handed;
};

MouseXdo::MouseXdo(int h, int v)
    : horizontal(h), vertical(v) {
    xdo = xdo_new(NULL);
    osu_x = data::cfg["resolution"]["width"].asInt();
    osu_y = data::cfg["resolution"]["height"].asInt();
    osu_h = data::cfg["resolution"]["horizontalPosition"].asInt();
    osu_v = data::cfg["resolution"]["verticalPosition"].asInt();
    is_letterbox = data::cfg["resolution"]["letterboxing"].asBool();
    is_left_handed = data::cfg["decoration"]["leftHanded"].asBool();
}

MouseXdo::~MouseXdo() {
    delete xdo;
}

std::unique_ptr<IMouse> create_mouse_handler(int h, int v) {
    return std::make_unique<MouseXdo>(h, v);
}

std::pair<double, double> MouseXdo::get_position() {
    double letter_x, letter_y, s_height, s_width;
    Window foreground_window;
    bool found_window = (xdo_get_focused_window_sane(xdo, &foreground_window) == 0);

    if (found_window) {
        unsigned char* name_ret;
        int name_len_ret;
        int name_type;

        xdo_get_window_name(xdo, foreground_window, &name_ret, &name_len_ret, &name_type);
        bool can_get_name = (name_len_ret > 0);

        if (can_get_name) {

            std::string title = "";

            if (name_ret != NULL)
            {
                std::string foreground_title(reinterpret_cast<char*>(name_ret));
                title = foreground_title;
            }

            if (title.find("osu!") == 0) {
                if (!is_letterbox) {

                    int x_ret;
                    int y_ret;
                    unsigned int width_ret;
                    unsigned int height_ret;

                    bool can_get_location = (xdo_get_window_location(xdo, foreground_window, &x_ret, &y_ret, NULL) == 0);
                    bool can_get_size = (xdo_get_window_size(xdo, foreground_window, &width_ret, &height_ret) == 0);

                    bool can_get_rect = (can_get_location && can_get_size);

                    bool is_fullscreen_window = (horizontal == width_ret) && (vertical == height_ret);
                    bool should_not_resize_screen = (!can_get_rect || is_fullscreen_window);

                    if (should_not_resize_screen) {
                        s_width = horizontal;
                        s_height = vertical;

                        letter_x = 0;
                        letter_y = 0;
                    }
                    else {
                        s_height = osu_y * 0.8;
                        s_width = s_height * 4 / 3;

                        long left = x_ret;
                        long top = y_ret;
                        long right = left + width_ret;
                        long bottom = top + height_ret;

                        letter_x = left + ((right - left) - s_width) / 2;
                        letter_y = top + osu_y * 0.117;
                    }
                }
                else {
                    s_height = osu_y * 0.8;
                    s_width = s_height * 4 / 3;

                    double l = (horizontal - osu_x) * (osu_h + 100) / 200.0;
                    double r = l + osu_x;
                    letter_x = l + ((r - l) - s_width) / 2;
                    letter_y = (vertical - osu_y) * (osu_v + 100) / 200.0 + osu_y * 0.117;
                }
            }
            else {
                s_width = horizontal;
                s_height = vertical;
                letter_x = 0;
                letter_y = 0;
            }
        }
        else {
            s_width = horizontal;
            s_height = vertical;
            letter_x = 0;
            letter_y = 0;
        }
    }
    else {
        s_width = horizontal;
        s_height = vertical;
        letter_x = 0;
        letter_y = 0;
    }

    double x = 0, y = 0;
    int px = 0, py = 0;

    if (xdo_get_mouse_location(xdo, &px, &py, NULL) == 0) {

        if (!is_letterbox) {
            letter_x = floor(1.0 * px / osu_x) * osu_x;
            letter_y = floor(1.0 * py / osu_y) * osu_y;
        }

        double fx = (1.0 * px - letter_x) / s_width;

        if (is_left_handed) {
            fx = 1 - fx;
        }

        double fy = (1.0 * py - letter_y) / s_height;

        fx = std::min(fx, 1.0);
        fx = std::max(fx, 0.0);

        fy = std::min(fy, 1.0);
        fy = std::max(fy, 0.0);

        x = fx;
        y = fy;
    }

    return std::make_pair(x, y);
}

}
