#include "header.hpp"
#include <memory>

int main(int argc, char ** argv) {
    sf::RenderWindow window;
    data::init();
    auto window_config = data::get_cfg()["window"];

    auto value_or = [](const Json::Value& v, int defv) {
        return v.isNull() ? defv : v.asInt();
    };

    const int window_width = value_or(window_config["size"][0], 612);
    const int window_height = value_or(window_config["size"][1], 352);

    window.create(sf::VideoMode(window_width, window_height), "Bongo Cat", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(MAX_FRAMERATE);
    std::unique_ptr<cats::ICat> cat;
    int mode;

    // loading configs
    while (!cat) {
        data::init();
        mode = data::get_cfg()["mode"].asInt();
        cat = cats::get_cat(mode);
    }

    // initialize input
    if (!input::init(window_width, window_height)) {
        return EXIT_FAILURE;
    }

    bool is_reload = false;
    bool is_show_input_debug = false;

    const int window_offset_x = value_or(window_config["offset"][0], 0);
    const int window_offset_y = value_or(window_config["offset"][1], 0);

    sf::Vector2f scene_pos;
    scene_pos.x = std::clamp(window_offset_x, 0, window_width);
    scene_pos.y = std::clamp(window_offset_y, 0, window_height);

    sf::Transform transform = sf::Transform();
    transform.translate(scene_pos);
    sf::RenderStates rstates = sf::RenderStates(transform);

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
                        cat.reset();
                        while (!cat) {
                            data::init();
                            mode = data::get_cfg()["mode"].asInt();
                            cat = cats::get_cat(mode);
                        }
                        
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
        cat->draw(window, rstates);

        if (is_show_input_debug) {
            input::drawDebugPanel(window);
        }

        window.display();
    }

    input::cleanup();
    return 0;
}

