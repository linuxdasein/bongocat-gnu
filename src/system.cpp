#include <system.hpp>
#include <filesystem>

namespace os
{

#if defined(__unix__) || defined(__unix)
// SystemInfo implementation for GNU/Linux systems
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

class SystemInfoGNU : public ISystemInfo
{
public:
    SystemInfoGNU() {
        const char *homedir;

        if ((homedir = getenv("HOME")) == NULL) {
            homedir = getpwuid(getuid())->pw_dir;
        }

        m_home_directory = std::string(homedir);
    }

    std::string get_config_file_path() const override {
        if(std::filesystem::exists(CONF_FILE_NAME)) {
            // If a file in the current exists it takes precedence
            return CONF_FILE_NAME;
        }
        else {
            // otherwise try to load a file from the user's home directory
            std::string config_file_path = m_home_directory + "/.config/bongocat-gnu/" + CONF_FILE_NAME;
            if(std::filesystem::exists(config_file_path)) {
                return config_file_path;
            }

            // if no file has been found, fallback to the default implementation
            return ISystemInfo::get_config_file_path();
        }
    }

private:
    std::string m_home_directory;
};

std::unique_ptr<ISystemInfo> create_system_info() {
    return std::make_unique<SystemInfoGNU>();
}
#else
std::unique_ptr<ISystemInfo> create_system_info() {
    return std::make_unique<ISystemInfo>();
}
#endif

}