#include <header.hpp>
#include <stdexcept>
#include <system.hpp>
#include <filesystem>

#define CONF_FILE_NAME "config.json"

namespace data {

ConfigFile::ConfigFile() 
   : conf_file_path(CONF_FILE_NAME) {}

bool ConfigFile::init(int argc, char ** argv) {
    const auto system_info = os::create_system_info();

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

    return true;
}

std::ifstream& ConfigFile::load_config_file() {
    cfg_file.open(conf_file_path);
    if (!cfg_file.good()) {
        std::string msg = "Error reading configs: Couldn't open config file " 
            + conf_file_path + ":\n";
        throw std::runtime_error(msg);
    }

    return cfg_file;
}

std::string ConfigFile::get_config_name() const {
    return conf_file_path;
}

}