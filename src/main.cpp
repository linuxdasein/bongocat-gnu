#include "cat.hpp"
#include "header.hpp"
#include "logger.hpp"
#include <algorithm>
#include <cstdlib>
#include <memory>

int main(int argc, char ** argv) {
    // initialize basic logging
    logger::GlobalLogger::init();

    if(!data::init()) {
        logger::error("Fatal error has occured during data initialization");
        return EXIT_FAILURE;
    }

    sf::RenderWindow window;
    // initially create window with default size
    sf::Vector2u window_size = data::g_window_default_size;
    window.create(sf::VideoMode(window_size), "Bongo Cat", sf::Style::Titlebar | sf::Style::Close);
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
    
    // load config file
    data::ConfigFile config_file;
    if (!config_file.init(argc, argv)) {
        logger::error("Fatal error has occured while loading config file");
        return EXIT_FAILURE;
    }

    data::Settings settings;
    std::unique_ptr<cats::ICat> cat;
    std::vector<std::string> modes;
    auto mode = modes.cend();
    sf::RenderStates rstates;

    auto reload_config = [&]() {
        // try to load config file
        if(!settings.reload(config_file))
            return false;

        // update cat modes list
        modes = settings.get_cat_modes();

        // get cat mode from the config
        auto cfg_mode_name = settings.get_default_mode();
        mode = std::find(modes.cbegin(), modes.cend(), cfg_mode_name);

        // initialize cat mode
        cat = std::make_unique<cats::CustomCat>();
        if (!cat->init(settings, settings.get_cat_config(cfg_mode_name)))
            return false;

        // update window transform data
        auto cfg_window_size = settings.get_window_size();
        if (window_size != cfg_window_size) {
            // reinitialize window only if config size has changed
            window_size = cfg_window_size;
            window.create(sf::VideoMode(window_size), 
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

        while (const std::optional event = window.pollEvent()) {
            if( event->is<sf::Event::Closed>() ) {
                window.close();
            }
            else if (const auto* evtKey = event->getIf<sf::Event::KeyPressed>()) {
                // get reload config prompt
                if (evtKey->code == sf::Keyboard::Key::R && evtKey->control) {
                    try_reload_config = true;
                    break;
                }

                // switch to the next cat mode
                if (evtKey->code == sf::Keyboard::Key::N && evtKey->control) {
                    if (is_config_loaded) {
                        ++mode;
                        if (mode == modes.end())
                            mode = modes.begin();
                        cat = std::make_unique<cats::CustomCat>();
                        is_config_loaded = cat->init(settings, settings.get_cat_config(*mode));
                    }
                    break;
                }

                // toggle joystick debug panel
                if (evtKey->code == sf::Keyboard::Key::D && evtKey->control) {
                    do_show_input_debug = !do_show_input_debug;
                    break;
                }

                // toggle log overlay
                if (evtKey->code == sf::Keyboard::Key::L && evtKey->control) {
                    do_show_debug_overlay = ! do_show_debug_overlay;
                    break;
                }
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

