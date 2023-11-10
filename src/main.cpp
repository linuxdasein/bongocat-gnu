#include "cats.hpp"
#include "header.hpp"
#include "logger.hpp"
#include <cstdlib>
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

static auto get_cat_mode(const std::string &s) {
    auto mode_it = std::find_if(modes.cbegin(), modes.cend(),
        [&s](const Mode& m) {return m.second == s;});

    if(mode_it == modes.cend()) {
        logger::error("Error reading configs: Mode value " + s + " is not correct");
    }

    return mode_it;
}

int main(int argc, char ** argv) {
    // initialize basic logging
    logger::GlobalLogger::init();

    if(!data::init()) {
        logger::error("Fatal error has occured. Exiting the application");
        return EXIT_FAILURE;
    }

    sf::RenderWindow window;
    // initially create window with default size
    sf::Vector2i window_size = data::get_cfg_window_default_size();
    window.create(sf::VideoMode(window_size.x, window_size.y), "Bongo Cat", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(MAX_FRAMERATE);

    // initialize input
    if (!input::init(window_size.x, window_size.y))
        return EXIT_FAILURE;
    
    // attach overlay logger
    auto p_log_overlay = std::make_unique<logger::SfmlOverlayLogger>(window_size.x, window_size.y);
    logger::SfmlOverlayLogger& log_overlay = *p_log_overlay.get();
    logger::GlobalLogger::get().attach(std::move(p_log_overlay));

    bool is_config_loaded = false;
    bool try_reload_config = true;
    bool is_show_input_debug = false;
    
    std::unique_ptr<cats::ICat> cat;
    auto mode = modes.end();
    sf::RenderStates rstates;

    auto reload_config = [&]() {
        // try to load config file
        if(!data::reload_config())
            return false;

        // get cat mode from the config
        auto cfg = data::get_cfg();

        // load cat mode
        mode = get_cat_mode(cfg["mode"].asString());
        if (mode == modes.end())
            return false;

        // initialize cat mode
        cat = cats::get_cat(mode->first);
        if (!cat->init(cfg))
            return false;

        // update window transform data
        auto cfg_window_size = data::get_cfg_window_size(cfg);
        if (window_size != cfg_window_size) {
            // reinitialize window only if config size has changed
            window_size = cfg_window_size;
            window.create(sf::VideoMode(window_size.x, window_size.y), 
                "Bongo Cat", sf::Style::Titlebar | sf::Style::Close);
            log_overlay.set_size(window_size);
            if (!input::init(window_size.x, window_size.y))
                return false;
        }

        // update windows transform
        sf::Transform transform = data::get_cfg_window_transform(cfg);
        rstates = sf::RenderStates(transform);

        return true;
    };

    while (window.isOpen()) {
        if (try_reload_config) {
            is_config_loaded = reload_config();
            try_reload_config = false;
        }

        sf::Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
            case sf::Event::Closed:
                window.close();
                break;

            case sf::Event::KeyPressed:
                // get reload config prompt
                if (event.key.code == sf::Keyboard::R && event.key.control) {
                    try_reload_config = true;
                    break;
                }

                // switch to the next cat mode
                if (event.key.code == sf::Keyboard::N && event.key.control) {
                    if (is_config_loaded) {
                        ++mode;
                        if (mode == modes.cend())
                            mode = modes.cbegin();
                        cat = cats::get_cat(mode->first);
                        is_config_loaded = cat->init(data::get_cfg());
                    }
                }

                // toggle joystick debug panel
                if (event.key.code == sf::Keyboard::D && event.key.control) {
                    is_show_input_debug = !is_show_input_debug;
                    break;
                }

            default: break;
            }
        }

        window.draw(log_overlay, rstates);

        if(!is_config_loaded) {
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

