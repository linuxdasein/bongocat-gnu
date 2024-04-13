#include <header.hpp>
#include <stdexcept>
#include <system.hpp>
#include <filesystem>
#include <cxxopts.hpp>

#define CONF_FILE_NAME "config.json"

namespace
{

std::optional<std::string> parse_cmd_options(int argc, char** argv) {
    cxxopts::Options opts("BongoCat", "Configurable Bongo cat overlay");

    opts.add_options()
        ("config", "Config file path", cxxopts::value<std::string>());

    opts.parse_positional("config");

    try {
        auto parsed_opts = opts.parse(argc, argv);
        return parsed_opts["config"].as<std::string>();
    }
    catch(cxxopts::exceptions::option_has_no_value&) {
        // valid case: no config file specified
    }

    return std::nullopt;
}

}

namespace data {

bool ConfigFile::init(int argc, char** argv) {
    std::optional<std::string> conf_file_opt;

    try { // try to get a config file from command line
        conf_file_opt = parse_cmd_options(argc, argv);
    }
    catch(cxxopts::exceptions::exception &e) {
        logger::info(std::string("Failed to parse arguments:") + e.what());
        return false;
    }
    
    conf_file_path = conf_file_opt.value_or(CONF_FILE_NAME);

    if(conf_file_opt.has_value()) {
        if(!std::filesystem::exists(*conf_file_opt)) {
            logger::error("Failed to open config file: " + *conf_file_opt);
            return false;
        }
        return true;
    }

    const auto system_info = os::create_system_info();

    // if a file exists in the current directory it takes precendence
    if(!std::filesystem::exists(conf_file_path)) {
        // otherwise try to load a file from user's home directory
        auto cfg_dir_path = system_info->get_config_dir_path();
        conf_file_path = cfg_dir_path / CONF_FILE_NAME;
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