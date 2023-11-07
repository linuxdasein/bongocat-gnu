#include "cats.hpp"
#include "header.hpp"
#include "logger.hpp"
#include <memory>
#include <stdexcept>

using Mode = std::pair<cats::CatModeId, std::string>;

static const std::array<Mode, 6> modes = {
    Mode{cats::CatModeId::osu,     "osu"},
    Mode{cats::CatModeId::taiko,   "taiko"},
    Mode{cats::CatModeId::ctb,     "ctb"},
    Mode{cats::CatModeId::mania,   "mania"},
    Mode{cats::CatModeId::custom,  "custom"},
    Mode{cats::CatModeId::classic, "classic"}
};

static auto init_cat() {
    auto mode_it = modes.cend();

    while(mode_it == modes.cend()) {
        auto s = data::get_cfg()["mode"].asString();
        mode_it = std::find_if(modes.cbegin(), modes.cend(),
            [&s](const Mode& m) {return m.second == s;});

        if(mode_it == modes.cend()) {
            std::string msg = "Error reading configs: Mode value " + s + " is not correct";
            logger::get().log(msg, logger::Severity::critical);
        }
    }

    return mode_it;
}

int main(int argc, char ** argv) {
    sf::RenderWindow window;
    data::init();
    
    sf::Vector2i window_size = data::get_cfg_window_size();
    logger::GlobalLogger::init(window_size.x, window_size.y);
    window.create(sf::VideoMode(window_size.x, window_size.y), "Bongo Cat", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(MAX_FRAMERATE);
    
    auto p_log_overlay = std::make_unique<logger::SfmlOverlayLogger>(window_size.x, window_size.y);
    logger::SfmlOverlayLogger& log_overlay = *p_log_overlay.get();
    logger::GlobalLogger::get().attach(std::move(p_log_overlay));

    // loading configs
    auto mode = init_cat();
    std::unique_ptr<cats::ICat> cat = cats::get_cat(mode->first);

    // initialize input
    if (!input::init(window_size.x, window_size.y)) {
        return EXIT_FAILURE;
    }

    bool is_reload = false;
    bool is_show_input_debug = false;

    sf::Transform transform = data::get_cfg_window_transform();
    sf::RenderStates rstates = sf::RenderStates(transform);

    bool is_cat_loaded = cat->init(data::get_cfg());

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
                        data::init();
                        mode = init_cat();
                        cat = cats::get_cat(mode->first);
                        is_cat_loaded = cat->init(data::get_cfg());
                    }
                    is_reload = true;
                    break;
                }

                // switch to the next cat mode
                if (event.key.code == sf::Keyboard::N && event.key.control) {
                    ++mode;
                    if (mode == modes.cend())
                        mode = modes.cbegin();
                    cat = cats::get_cat(mode->first);
                    cat->init(data::get_cfg());
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

        window.draw(log_overlay, rstates);

        if(!is_cat_loaded) {
            window.display();
            continue;
        }
        else {
            log_overlay.set_visible(false);
        }

        Json::Value rgb = data::get_cfg()["decoration"]["rgb"];
        int red_value = rgb[0].asInt();
        int green_value = rgb[1].asInt();
        int blue_value = rgb[2].asInt();
        int alpha_value = rgb.size() == 3 ? 255 : rgb[3].asInt();

        window.clear(sf::Color(red_value, green_value, blue_value, alpha_value));
        cat->update();
        window.draw(*cat, rstates);

        if (is_show_input_debug) {
            input::drawDebugPanel(window);
        }

        window.display();
    }

    input::cleanup();
    return 0;
}

