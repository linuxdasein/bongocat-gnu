#include <system.hpp>

namespace os
{

// SystemInfo implementation for GNU/Linux systems
#include <unistd.h>
#include <sys/types.h>
#include <linux/limits.h>
#include <pwd.h>

class SystemInfoGNU : public ISystemInfo
{
public:

    std::filesystem::path get_config_dir_path() const override {
        const char *homedir;

        if ((homedir = getenv("HOME")) == NULL) {
            homedir = getpwuid(getuid())->pw_dir;
        }

        return std::filesystem::path(homedir) / ".config/bongocat-gnu/";
    }

    std::filesystem::path get_app_dir_path() const override {
        char exe_path[PATH_MAX];

        int nchar = readlink("/proc/self/exe", exe_path, sizeof(exe_path));

        if (nchar < 0 || nchar >= (int)sizeof(exe_path))
            return "";

        std::filesystem::path fs_path(std::string(exe_path, nchar));

        return fs_path.parent_path().parent_path();
    }
};

std::unique_ptr<ISystemInfo> create_system_info() {
    return std::make_unique<SystemInfoGNU>();
}

}