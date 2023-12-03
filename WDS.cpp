#include "WDS.h"

std::vector<std::string> wds::customSplit(std::string str, char separator) {
    std::vector<std::string> strings;
    int startIndex = 0, endIndex = 0;
    for (int i = 0; i <= str.size(); i++) {
        
        // If we reached the end of the word or the end of the input.
        if (str[i] == separator || i == str.size()) {
            endIndex = i;
            std::string temp;
            temp.append(str, startIndex, endIndex - startIndex);
            strings.push_back(temp);
            startIndex = endIndex + 1;
        }
    }

    return strings; 
}


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

std::vector<Aux::checked_package> wds::has_active_dependencies(std::set<std::string> packagesNames, Cacher& ch) {
    std::vector<Aux::checked_package> out;
    for (auto packNames: packagesNames) {
        std::set<std::string> was;
        bool can_delete = !_has_active_dependencies(packNames, ch, was);
        // out.emplace_back(packNames, 200, can_delete);
        out.push_back({packNames, 200, can_delete});
    }

    return out;
} 

bool wds::_has_active_dependencies(std::string package_name, Cacher& ch, std::set<std::string>& was) {
    was.insert(package_name);
    auto value = ch.getCache(package_name);
    if (value.has_value()) {
        return !value.value().can_delete;
    }
    
    if (has_aptitude_dependencies(package_name)) {
        ch.setCache(package_name, false);
        return true;
    }

    auto provides = get_package_provides(package_name);
    for (auto p: provides) {
        if (!was.count(p) && _has_active_dependencies(p, ch, was)) {
            ch.setCache(p, false);
            return true;
        }
    }

    ch.setCache(package_name, true);
    return false;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// TODO сделать проверку по бранчу, который задает пользователь, а не по тому, который в системе
bool wds::has_aptitude_dependencies(std::string package_name) {
    const char* apt_cmd = std::string("aptitude search '~D" + package_name + "'").c_str();
    auto dependencies = exec(apt_cmd);
    return !dependencies.empty();
}

std::set<std::string> wds::get_package_provides(std::string package_name) {

    if (virtual_packages.count(package_name)) {
        return virtual_parents[package_name];
    }

    return actual_provides[package_name];
}