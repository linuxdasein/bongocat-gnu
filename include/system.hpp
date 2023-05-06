// Generic interface for features that have platform-specific implementation 

#pragma once

#define CONF_FILE_NAME "config.json"

#include <string>
#include <memory>

namespace os
{

class ISystemInfo
{
public:

    // Get the OS dependent config file location
    virtual std::string get_config_file_path() const {
        // By default load from the current directory
        return CONF_FILE_NAME;
    };
};

std::unique_ptr<ISystemInfo> create_system_info();

}
