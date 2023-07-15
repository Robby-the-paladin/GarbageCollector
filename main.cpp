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
    
    set<string> keywords;
    keywords.insert("Obsoletes:");
    keywords.insert("Provides:");

    string branch = "p10";
    auto pnames = s.getBranchPackageNames(branch);

    for (auto pname : pnames) {
        usleep(5000);
        string spec = s.getSpec(branch, pname);
        cout << "\nIn package " << pname << ":\n";
        auto packages = p.getDeprecatedPackages(spec);
        for (auto pack : packages) {
            cout << pack << endl;
        }

    }

    return 0;
}