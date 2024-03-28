#include "cats.hpp"
#include "header.hpp"
#include "logger.hpp"
#include <cstdlib>
#include <memory>
#include <stdexcept>

using Mode = std::pair<cats::CatModeId, std::string>;

static std::vector<Mode> modes = {
    Mode{cats::CatModeId::osu,     "osu"},
    Mode{cats::CatModeId::taiko,   "taiko"},
    Mode{cats::CatModeId::ctb,     "ctb"},
    Mode{cats::CatModeId::mania,   "mania"}
};

static auto get_cat_mode(const std::string &s) {
    auto mode_it = std::find_if(modes.begin(), modes.end(),
        [&s](const Mode& m) {return m.second == s;});

    return mode_it;
}

int main(int argc, char ** argv) {
    // initialize basic logging
    logger::GlobalLogger::init();

    if(!data::init()) {
        logger::error("Fatal error has occured during data initialization");
        return EXIT_FAILURE;
    }

    sf::RenderWindow window;
    // initially create window with default size
    sf::Vector2i window_size = data::g_window_default_size;
    window.create(sf::VideoMode(window_size.x, window_size.y), "Bongo Cat", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(MAX_FRAMERATE);

    // initialize input
    if (!input::init(window_size.x, window_size.y)) {
        logger::error("Fatal error has occured during input initialization");
        return EXIT_FAILURE;
    }
    
    // attach overlay logger
    auto p_log_overlay = std::make_unique<logger::SfmlOverlayLogger>(window_size.x, window_size.y);
    logger::SfmlOverlayLogger& log_overlay = *p_log_overlay.get();
    logger::GlobalLogger::get().attach(std::move(p_log_overlay));

    logger::info("Bongocat overlay logger has been sucsessfully attached");

    bool is_config_loaded = false;
    bool try_reload_config = true;
    bool do_show_input_debug = false;
    bool do_show_debug_overlay = false;
    
    data::Settings settings;
    std::unique_ptr<cats::ICat> cat;
    auto mode = modes.end();
    sf::RenderStates rstates;

    auto reload_config = [&]() {
        // try to load config file
        if(!settings.reload())
            return false;

        // update cat modes list
        auto cfg_modes = settings.get_cat_modes();
        for (auto m : cfg_modes) {
            if (modes.end() != get_cat_mode(m)) {
                logger::error("mode " + m + " is already defined");
                return false;
            }
            modes.push_back(Mode{cats::CatModeId::custom, m});
        }

        // get cat mode from the config
        auto cfg_mode_name = settings.get_default_mode();
        mode = get_cat_mode(cfg_mode_name);
        if (mode == modes.end()) {
            logger::error("Error reading configs: Mode value " + cfg_mode_name + " is not correct");
            return false;
        }

        // initialize cat mode
        cat = cats::get_cat(mode->first);
        if (!cat->init(settings, settings.get_cat_config(mode->second)))
            return false;

        // update window transform data
        auto cfg_window_size = settings.get_window_size();
        if (window_size != cfg_window_size) {
            // reinitialize window only if config size has changed
            window_size = cfg_window_size;
            window.create(sf::VideoMode(window_size.x, window_size.y), 
                "Bongo Cat", sf::Style::Titlebar | sf::Style::Close);
            log_overlay.set_size(window_size);
            if (!input::init(window_size.x, window_size.y, settings.is_mouse_left_handed()))
                return false;
        }

        // update windows transform
        sf::Transform transform = settings.get_window_transform();
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
                        if (mode == modes.end())
                            mode = modes.begin();
                        cat = cats::get_cat(mode->first);
                        is_config_loaded = cat->init(settings, settings.get_cat_config(mode->second));
                    }
                    break;
                }

                // toggle joystick debug panel
                if (event.key.code == sf::Keyboard::D && event.key.control) {
                    do_show_input_debug = !do_show_input_debug;
                    break;
                }

                // toggle log overlay
                if (event.key.code == sf::Keyboard::L && event.key.control) {
                    do_show_debug_overlay = ! do_show_debug_overlay;
                    break;
                }

            default: break;
            }
        }

        if(!is_config_loaded) {
            window.draw(log_overlay, rstates);
            window.display();
            continue;
        }
        else {
            log_overlay.set_visible(do_show_debug_overlay);
        }

        window.clear(settings.get_background_color());
        cat->update();
        window.draw(*cat, rstates);

        window.draw(log_overlay, rstates);

        if (do_show_input_debug) {
            input::drawDebugPanel(window);
        }

        window.display();
    }

    input::cleanup();
    return 0;
}

