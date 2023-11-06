#include <iostream>
#include <regex>
#include <unistd.h>
#include <set>
#include <map>
#include <queue>
#include <thread>
#include <mutex>
#include "SpecCollector.h"
#include "SpecParser.h"
#include "PostgreHandler.h"
#include "Api.h"
#include <rpm/header.h>
#include "rpmDB_test.h"
#include "LegacyDependencyAnalyzer.h"
#include "PatchMaker.h"
//#include <pqxx/pqxx>

using namespace std;

int counter = 0;
mutex cnt_mute;
// А точно безоавсно хранить админский ключ в строке на гите, и так сойдет ...
std::string postgreConnStr = "user=doadmin password=AVNS_xMD70wwF41Btbfo6iaz host=db-postgresql-fra1-79796-do-user-14432859-0.b.db.ondigitalocean.com port=25060 dbname=test target_session_attrs=read-write";
//std::string postgreConnStr = "user=edgar password=genius host=host.docker.internal port=5432 dbname=test target_session_attrs=read-write";
//Прокся
//std::string apiURL = "http://64.226.73.174:8080";
//не прокся (медленно)
std::string apiURL = "https://rdb.altlinux.org";
int threadsSize = 100;

std::set<std::string> errorPackages;

std::ostream& operator << (std::ostream &os, const SpecParser::lib_data &lib)
{
    return os << lib.name << " " << lib.sign << " " << lib.version << " " << lib.type;
}

PostgreHandler ph;

std::set<std::string> unic_list;

set<std::string> getUnicalPackageNames(vector<std::string> fromApi, set<std::string> fromDB){
    std::set<std::string> remaining_names;

    for (const std::string& name : fromApi) {
        if (fromDB.find(name) == fromDB.end()) {
            remaining_names.insert(name);
        }
    }
    return remaining_names;
}



int main(int argc, char *argv[]) {

    // Api::getBranchPackageNames("sisyphus");
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
        if (*it != "asterisk-core-sounds-fr-gsm") {
            continue;
        }
        test.insert(*it);
        index++;
    }

    L.analysingBranchPackages(test);
    // L.packagesToAnalyse = t;

    std::vector<std::string> packages;
    for(auto pack: L.packagesToAnalyse) {
        cout << pack.second << endl;
        packages.push_back(pack.second);
    }
    // return 0;

    std::cout << L.packagesToAnalyse.size() << std::endl;

    auto P = PatchMaker();
    P.packagesToPatch = packages;
    P.dependenciesToDelete = L.criteriaChecking();
    P.loadSpecs(PatchMaker::specLoader::apiLoader);
    P.makePatch("./Patches/");

    return 0;
}
