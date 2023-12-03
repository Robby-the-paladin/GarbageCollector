#include "WDS.h"

std::string wds::exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

bool wds::has_active_dependencies(std::string package_name) {
    if (has_aptitude_dependencies(package_name))
        return true;

    auto provides = get_package_provides(package_name);    
    for (int i = 0; i < provides.size(); i++) {
        if (has_active_dependencies(provides[i])) {
            return true;
        }
    }
}

bool wds::has_aptitude_dependencies(std::string package_name) {
    char* apt_cmd = std::string("aptitude search '~D" + package_name + "'").c_str();
    auto dependencies = exec(apt_cmd);
    return dependencies.empty();
}

std::vector<std::string> get_package_provides(std::string package_name) {}