#pragma once

#include <string>
#include <set>
#include <vector>
#include <memory>
#include <unistd.h>
#include <array>

namespace Aux {
     struct checked_package {
        // название пакета
        std::string name;
        // код ответа севрера
        long http_code;
        // true если никем не используется, иначе false
        bool can_delete;
    };
    inline std::set<std::string> virtual_packages;

    inline bool is_virtual(std::string packName) {
        return virtual_packages.count(packName);
    }

    inline std::vector<std::string> custom_split(std::string str, char separator) {
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

    inline std::string exec(const char* cmd) {
        std::array<char, 128> buffer;
        std::string result;
        std::cout << "RUN CMD: " << cmd << std::endl;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
        if (!pipe) {
            throw std::runtime_error("popen() failed!");
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        return result;
    }

    inline void init() {
        std::string apt_cmd = std::string("aptitude search '~v'");
        auto dependencies = custom_split(exec(apt_cmd.c_str()), '\n');
        for (auto dep: dependencies) {
            for (auto elem: custom_split(dep, ' ')) {
                virtual_packages.insert(elem);
            }
        }
    }
}