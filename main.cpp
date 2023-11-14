#include <iostream>
#include <unistd.h>
#include <set>
#include <map>
#include "Api.h"
#include "LegacyDependencyAnalyzer.h"
#include "PatchMaker.h"
#include "Cacher.h"

using namespace std;

// А точно безоавсно хранить админский ключ в строке на гите, и так сойдет ...
std::string postgreConnStr = "user=doadmin password=AVNS_xMD70wwF41Btbfo6iaz host=db-postgresql-fra1-79796-do-user-14432859-0.b.db.ondigitalocean.com port=25060 dbname=test target_session_attrs=read-write";
//std::string postgreConnStr = "user=edgar password=genius host=host.docker.internal port=5432 dbname=test target_session_attrs=read-write";
//Прокся
//std::string apiURL = "http://64.226.73.174:8080";
//не прокся (медленно)
std::string apiURL = "https://rdb.altlinux.org";
int threadsSize = 100;

std::set<std::string> errorPackages;



int main(int argc, char *argv[]) {

    // Api::getBranchPackageNames("sisyphus");
    Cacher CH;

    auto  L = LegacyDependencyAnalyzer();
    
    auto t = Api::getBranchPackageNames("sisyphus");
    // t.resize(10);
    set<std::string> test;
    int index = 0;
    for (auto it = t.begin(); it != t.end(); it++) {
        // if (index <= 20) {
        //     index++;
        //     continue;
        // } 
        // if (index >= 40) {
        //     break;
        // }
        // if (*it != "asterisk-core-sounds-fr-gsm" && *it != "asterisk-core-sounds-fr-siren14" &&
            // *it != "ocaml-sqlite3" && *it != "asterisk-core-sounds-fr-sln16" &&
            // *it != "samba-client-control" && *it != "alterator-wizardface" && 
        if(
            (*it).substr(0, string("asterisk-core-sounds").size()) != "asterisk-core-sounds") {
            continue;
        }
        test.insert(*it);
        index++;
    }

    L.analysingBranchPackages(test);
    // L.packagesToAnalyse = t;

    //  for(auto pack: L.packagesToAnalyse) {
    //     cout << pack.second.first << " Release: " << pack.second.second << endl;
    // }

    // return 0;

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
