#include <iostream>
#include <unistd.h>
#include <set>
#include <regex>
#include <map>
#include "Api.h"
#include "LegacyDependencyAnalyzer.h"
#include "PatchMaker.h"
#include "Cacher.h"
#include "Config.h"


using namespace std;

// А точно безоавсно хранить админский ключ в строке на гите, и так сойдет ...
std::string postgreConnStr = "";
//Прокся
//std::string apiURL = "http://64.226.73.174:8080";
//не прокся (медленно)
std::string apiURL = "https://rdb.altlinux.org";

// возвращает true если пакет подходит хотя бы под одно регулярное выражение из конфига
bool packageExamined(std::string pack, const std::vector<std::string>& pack_regexs) {
    for (auto r: pack_regexs) {
        std::regex R(r);
        std::smatch cm;
        if (std::regex_search(pack, cm, R)) {
            return true;
        }
    }
    return false;
}

int main(int argc, char *argv[]) {
    Config cf;
    postgreConnStr = cf.getConnectDB();
    auto pack_regexs = cf.getPackRegexs();

    std::cout << postgreConnStr << std::endl;
    Cacher CH;

    auto  L = LegacyDependencyAnalyzer();
    
    auto t = Api::getBranchPackageNames("sisyphus");
    set<std::string> test;
    int index = 0;
    for (auto it = t.begin(); it != t.end(); it++) {
        if (!packageExamined(*it, pack_regexs)) {
            continue;
        }

        test.insert(*it);
        index++;
    }

    L.analysingBranchPackages(test);
 

    std::vector<std::string> packages;
    std::map<std::string, std::pair<std::string, std::string>> test_pack;
    int count = 5;
    for(auto pack: L.packagesToAnalyse) {
        cout << pack.second.first << endl;
        packages.push_back(pack.second.first);
        test_pack[pack.first] = pack.second;
        if (count <= 0) {
            L.packagesToAnalyse = test_pack;
            break;
        }
        count--;
    }
    // return 0;
    // packages.resize(5);
    std::cout << L.packagesToAnalyse.size() << std::endl;

    auto P = PatchMaker();
    P.packagesToPatch = packages;
    P.dependenciesToDelete = L.criteriaChecking(CH);
    P.loadSpecs(PatchMaker::specLoader::apiLoader);
    P.makePatch("./Patches2/");

    return 0;
}
