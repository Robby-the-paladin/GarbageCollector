#pragma once

#include <iostream>
#include <memory>
#include <array>
#include <unistd.h>
#include <set>
#include <map>
#include <vector>

class wds {
public:
    static bool has_active_dependencies(std::string package_name);

    static bool has_aptitude_dependencies(std::string package_name);

    static std::vector<std::string> get_package_provides(std::string package_name);
private:
    static std::string exec(const char* cmd);
};