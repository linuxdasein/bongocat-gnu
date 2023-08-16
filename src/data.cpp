#include "header.hpp"
#include <memory>
#include <system.hpp>
#include <filesystem>
#define BONGO_ERROR 1

#include <unistd.h>
#include <limits.h>

extern "C" {
#include <SDL2/SDL.h>
}

namespace data {
Json::Value cfg;
std::map<std::string, sf::Texture> img_holder;

std::unique_ptr<Json::Value> parse_config_file(std::ifstream& cfg_file) {
    std::string cfg_string((std::istreambuf_iterator<char>(cfg_file)), std::istreambuf_iterator<char>()), error;
    Json::CharReaderBuilder cfg_builder;
    auto cfg = std::make_unique<Json::Value>();
    Json::CharReader *cfg_reader = cfg_builder.newCharReader();

    if (!cfg_reader->parse(cfg_string.c_str(), cfg_string.c_str() + cfg_string.size(), cfg.get(), &error)) {
        delete cfg_reader;
        throw std::runtime_error(error);
    } 
        
    delete cfg_reader;

    return cfg;
}

const Json::Value& get_cfg() {
    return cfg;
}

void error_msg(std::string error, std::string title) {
    SDL_MessageBoxButtonData buttons[] = {
        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Retry" },
        { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "Cancel" },
    };

    SDL_MessageBoxColorScheme colorScheme = {
        { /* .colors (.r, .g, .b) */
     /* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
        { 255, 255,255 },
        /* [SDL_MESSAGEBOX_COLOR_TEXT] */
        { 0, 0, 0 },
        /* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
        { 0, 0, 0 },
        /* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
        { 255,255, 255 },
        /* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
        { 128, 128, 128 }
        }
    };

    SDL_MessageBoxData messagebox_data = {
    	SDL_MESSAGEBOX_ERROR,
    	NULL,
    	title.c_str(),
    	error.c_str(),
    	SDL_arraysize(buttons),
    	buttons,
    	&colorScheme
    };

    int button_id;

    SDL_ShowMessageBox(&messagebox_data, &button_id);

    if (button_id == -1 || button_id == 1) {
        exit(BONGO_ERROR);
    }
}

bool update(Json::Value &cfg_default, Json::Value &cfg) {
    bool is_update = true;
    for (const auto &key : cfg.getMemberNames()) {
        if (cfg_default.isMember(key)) {
            if (cfg_default[key].type() != cfg[key].type()) {
                error_msg("Value type error in " CONF_FILE_NAME, "Error reading configs");
                return false;
            }
            if (cfg_default[key].isArray() && !cfg_default[key].empty()) {
                for (Json::Value &v : cfg[key]) {
                    if (v.type() != cfg_default[key][0].type()) {
                        error_msg("Value type in array error in " CONF_FILE_NAME, "Error reading configs");
                        return false;
                    }
                }
            }
            if (cfg_default[key].isObject()) {
                is_update &= update(cfg_default[key], cfg[key]);
            } else {
                cfg_default[key] = cfg[key];
            }
        } else {
            cfg_default[key] = cfg[key];
        }
    }
    return is_update;
}

void init() {
    const auto system_info = os::create_system_info();

    while (true) {
        std::string conf_file_path = CONF_FILE_NAME;
        // if a file exists in the current directory it takes precendence
        if(!std::filesystem::exists(conf_file_path)) {
            // otherwise try to load a file from user's home directory
            auto cfg_dir_path = system_info->get_config_dir_path();
            conf_file_path = cfg_dir_path + CONF_FILE_NAME;
            if(!std::filesystem::exists(conf_file_path)) {
                // if no config file is present, create one with the default settings
                const std::string cfg_file_template_path = "share/" CONF_FILE_NAME;
                
                if (std::filesystem::exists(cfg_file_template_path)) {
                    // create config from the default config template
                    std::filesystem::create_directories(cfg_dir_path);
                    std::filesystem::copy(cfg_file_template_path, conf_file_path);
                }
            }
        }
        std::ifstream cfg_file(conf_file_path);
        if (!cfg_file.good()) {
            std::string msg = "Couldn't open config file " + conf_file_path + ":\n";
            error_msg( msg, "Error reading configs");
            continue;
        }

        std::unique_ptr<Json::Value> cfg_read;
        try {
            cfg_read = parse_config_file(cfg_file);
        }
        catch(std::runtime_error& e) {
            std::string msg = "Syntax error in " CONF_FILE_NAME ":\n";
            error_msg( msg + e.what(), "Error reading configs");
            continue;
        }

        if (update(cfg, *cfg_read)) {
            break;
        }
    }

    img_holder.clear();
}

sf::Texture &load_texture(std::string path) {
    if (img_holder.find(path) == img_holder.end()) {
        while (!img_holder[path].loadFromFile(path)) {
            error_msg("Cannot find file " + path, "Error importing images");
        }
    }
    return img_holder[path];
}
}; // namespace data
