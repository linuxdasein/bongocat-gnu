#include "header.hpp"
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

const char *create_config() {
    const char *s =
        R"V0G0N({
    "mode": 1,
    "resolution": {
        "letterboxing": false,
        "width": 1920,
        "height": 1080,
        "horizontalPosition": 0,
        "verticalPosition": 0
    },
    "decoration": {
        "leftHanded": false,
        "rgb": [255, 255, 255],
        "offsetX": [0, 11],
        "offsetY": [0, -65],
        "scalar": [1.0, 1.0]
    },
    "osu": {
        "mouse": true,
        "toggleSmoke": false,
        "paw": [255, 255, 255],
        "pawEdge": [0, 0, 0],
        "key1": [90],
        "key2": [88],
        "smoke": [67],
        "wave": []
    },
    "taiko": {
        "leftCentre": [88],
        "rightCentre": [67],
        "leftRim": [90],
        "rightRim": [86]
    },
    "catch": {
        "left": [37],
        "right": [39],
        "dash": [16]
    },
    "mania": {
        "4K": true,
        "key4K": [68, 70, 74, 75],
        "key7K": [83, 68, 70, 32, 74, 75, 76]
    },
    "custom": {
        "background": "img/osu/mousebg.png",
        "mouse": false,
        "mouseImage": "img/osu/mouse.png",
        "mouseOnTop": true,
        "offsetX": 0,
        "offsetY": 0,
        "scalar": 1.0,
        "paw": [255, 255, 255],
        "pawEdge": [0, 0, 0],
        "keyContainers": []
    },
    "mousePaw": {
        "mousePawComment": "coordinates start in the top left of the window",
        "pawStartingPoint": [211, 159],
        "pawEndingPoint": [258, 228]
    }
})V0G0N";
    std::string error;
    Json::CharReaderBuilder cfg_builder;
    Json::CharReader *cfg_reader = cfg_builder.newCharReader();
    cfg_reader->parse(s, s + strlen(s), &cfg, &error);
    delete cfg_reader;

    return s;
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

bool init() {
    while (true) {
        const char* default_config = create_config();
        auto system_info = os::create_system_info();
        std::string conf_file_path = CONF_FILE_NAME;
        // if a file exists in the current directory it takes precendence
        if(!std::filesystem::exists(conf_file_path)) {
            // otherwise try to load a file from user's home directory
            conf_file_path = system_info->get_config_dir_path() + CONF_FILE_NAME;
            if(!std::filesystem::exists(conf_file_path)) {
                // if no config file is present, create one with the deault settings
                std::ofstream crg_file_o(conf_file_path, std::ifstream::binary);
                crg_file_o << default_config;
            }
        }
        std::ifstream cfg_file(conf_file_path, std::ifstream::binary);
        if (!cfg_file.good()) {
            break;
        }
        std::string cfg_string((std::istreambuf_iterator<char>(cfg_file)), std::istreambuf_iterator<char>()), error;
        Json::CharReaderBuilder cfg_builder;
        Json::CharReader *cfg_reader = cfg_builder.newCharReader();
        Json::Value cfg_read;
        if (!cfg_reader->parse(cfg_string.c_str(), cfg_string.c_str() + cfg_string.size(), &cfg_read, &error)) {
            delete cfg_reader;
            error_msg("Syntax error in " CONF_FILE_NAME ":\n" + error, "Error reading configs");
        } else if (update(cfg, cfg_read)) {
            delete cfg_reader;
            break;
        }
    }

    img_holder.clear();

    int mode = data::cfg["mode"].asInt();

    switch (mode) {
    case 1:
        return osu::init();
    case 2:
        return taiko::init();
    case 3:
        return ctb::init();
    case 4:
        return mania::init();
    case 5:
        return custom::init();
    default:
        error_msg("Mode value is not correct", "Error reading configs");
        return false;
    }
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
