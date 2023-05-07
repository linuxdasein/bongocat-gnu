#include <system.hpp>

namespace os
{

// SystemInfo implementation for GNU/Linux systems
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

class SystemInfoGNU : public ISystemInfo
{
public:

    std::string get_config_dir_path() const override {
        const char *homedir;

        if ((homedir = getenv("HOME")) == NULL) {
            homedir = getpwuid(getuid())->pw_dir;
        }

        return std::string(homedir) + "/.config/bongocat-gnu/";
    }
};

std::unique_ptr<ISystemInfo> create_system_info() {
    return std::make_unique<SystemInfoGNU>();
}

}