#include <iostream>
#include <regex>
#include <set>
#include "SpecCollector.h"
#include "Api.h"

using namespace std;

int main() {
    std::cout << "Edgar genius!" << std::endl;
    SpecCollector s;
    Api api;
    
    set<string> keywords;
    keywords.insert("Obsoletes:");
    keywords.insert("Provides:");

    string branch = "p10";
    auto pnames = s.getBranchPackageNames(branch);

    cout << pnames.size() * 500 / 1000 / 3600 << endl;
    /*for (auto pname : pnames) {
        Sleep(500);
        string spec = s.getSpec(branch, pname);
        cout << "\nIn package " << pname << ":\n";
        for (auto keyword : keywords) {
            string ex = "(" + keyword + ") (.*) (=|<|<=|<=|>=) (.*)";
            regex expr(ex, regex::icase);
            string::const_iterator searchStart(spec.cbegin());
            smatch res;
            while (regex_search(searchStart, spec.cend(), res, expr))
            {
                cout << (searchStart == spec.cbegin() ? "" : "\n") << res[0] << endl;
                searchStart = res.suffix().first;
            }
        }
    }*/

    return 0;
}