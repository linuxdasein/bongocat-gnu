#include "header.hpp"

sf::RenderWindow window;

int main(int argc, char ** argv) {

    window.create(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Bongo Cat for osu!", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(MAX_FRAMERATE);

    // loading configs
    while (!data::init()) {
        continue;
    }

    // initialize input
    if (!input::init()) {
        return EXIT_FAILURE;
    }

    bool is_reload = false;
    bool is_show_input_debug = false;

    int mode = data::get_cfg()["mode"].asInt();
    auto cat = cats::get_cat(mode);
    cat->init(data::get_cfg());

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
            case sf::Event::Closed:
                window.close();
                break;

            case sf::Event::KeyPressed:
                // get reload config prompt
                if (event.key.code == sf::Keyboard::R && event.key.control) {
                    if (!is_reload) {
                        while (!data::init()) {
                            continue;
                        }
                        mode = data::get_cfg()["mode"].asInt();
                        cat = cats::get_cat(mode);
                        cat->init(data::get_cfg());
                    }
                    is_reload = true;
                    break;
                }

                // toggle joystick debug panel
                if (event.key.code == sf::Keyboard::D && event.key.control) {
                    is_show_input_debug = !is_show_input_debug;
                    break;
                }

            default:
                is_reload = false;
            }
        }

        Json::Value rgb = data::get_cfg()["decoration"]["rgb"];
        int red_value = rgb[0].asInt();
        int green_value = rgb[1].asInt();
        int blue_value = rgb[2].asInt();
        int alpha_value = rgb.size() == 3 ? 255 : rgb[3].asInt();

        window.clear(sf::Color(red_value, green_value, blue_value, alpha_value));
        cat->draw();

        if (is_show_input_debug) {
            input::drawDebugPanel();
        }

        window.display();
    }

    input::cleanup();
    return 0;
}

