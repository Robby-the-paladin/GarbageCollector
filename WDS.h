#pragma once

#include <iostream>
#include <memory>
#include <array>
#include <unistd.h>
#include <set>
#include <map>
#include <vector>
#include "Cacher.h"
#include "Aux.h"
#include "RpmHandler.h"

class wds {
public:
    static void init(std::string folderClassicFiles, std::vector<std::string> oldBranches, std::string constNameClassic, std::vector<std::string> classicArches) {
        // for (auto br: oldBranches)
        // {
        //     for(auto arch: classicArches) {
        //         auto getPack = RpmHandler::getAllProvides(folderClassicFiles, br, constNameClassic, arch);
        //         old_provides.insert(getPack.begin(), getPack.end());
        //     }
        // }
        
        // for (auto arch: classicArches) {
        //     for (auto act_p: RpmHandler::getAllProvides("./", "", constNameClassic, arch)) {
        //         old_provides.erase(act_p);
        //     }
        // }

        actual_provides = RpmHandler::packagesProvides();

        for (auto pack: actual_provides) {
            for(auto pack_p: pack.second) {
                virtual_parents[pack_p].insert(pack.first);
            }
        }
    };

    static std::vector<Aux::checked_package> has_active_dependencies(std::vector<std::string> packagesNames, Cacher& ch);

    static bool has_aptitude_dependencies(std::string package_name);

    static std::set<std::string> get_package_provides(std::string package_name);
private:
    static inline std::map<std::string, std::set<std::string>> actual_provides;
    static inline std::map<std::string, std::set<std::string>> virtual_parents;
    static inline std::set<std::string> old_provides;
    // static inline std::set<std::string> virtual_packages;

    static bool _has_active_dependencies(std::string package_name,
                                         Cacher& ch, std::set<std::string>& was);
};