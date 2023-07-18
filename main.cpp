#include <iostream>
#include <regex>
#include <unistd.h>
#include <set>
#include "SpecCollector.h"
#include "SpecParser.h"
#include "Api.h"

using namespace std;

int main() {
    std::cout << "Edgar genius!" << std::endl;
    SpecCollector s;
    SpecParser p;
    Api api;
    /*
    set<string> keywords;
    keywords.insert("Obsoletes:");
    keywords.insert("Provides:");

    string branch = "p10";
    auto pnames = s.getBranchPackageNames(branch);

    for (auto pname : pnames) {
        usleep(50000);
        string spec = s.getSpec(branch, pname);
        cout << "\nIn package " << pname << ":\n";
        auto packages = p.getDeprecatedPackages(spec);
        for (auto pack : packages) {
            cout << pack << endl;
        }

    }
    */
    // std::string req = "https://rdb.altlinux.org/api/export/repology/p10";
    // req = "https://rdb.altlinux.org/api/export/branch_binary_packages/p10?arch=x86_64";
    // Json::Value list_p = Api::getReadBuffer(req); //["packages"];
    // std::cout << list_p << std::endl;
    /*
    for (auto p : list_p) {
        std::cout << "Package name source: " << p["name"] << " Binary: " << p["binaries"][0]["name"] << std::endl;
    }
    */
    string packageName = "boost-atomic-devel";
    cout << "Package name: " << packageName << "\n" << Api::checkPackage(packageName) <<endl;
    return 0;
}