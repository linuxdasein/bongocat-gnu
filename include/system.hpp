// Generic interface for features that have platform-specific implementation 

#pragma once

#include <memory>
#include <filesystem>

namespace os
{

class ISystemInfo
{
public:

    // Get the OS dependent user's config file location
    virtual std::filesystem::path get_config_dir_path() const {
        // By default use the current directory
        return "";
    };

    // Get full path to the application installation dir
    virtual std::filesystem::path get_app_dir_path() const {
        // By default use the current directory
        return "";
    };

    virtual ~ISystemInfo() {}
};

std::unique_ptr<ISystemInfo> create_system_info();

}
