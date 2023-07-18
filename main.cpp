#include <iostream>
#include <regex>
#include <unistd.h>
#include <set>
#include "SpecCollector.h"
#include "SpecParser.h"
#include "Api.h"
#include <pqxx/pqxx>

using namespace std;

//#define first_buld

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

int main() {
    try {
    // Connect to the database
        pqxx::connection c("user=edgar password=genius host=host.docker.internal port=5432 dbname=test target_session_attrs=read-write");
    } catch (const exception &e) {
        cerr << e.what() << endl;
     //   return 1;
    }
    //return 0;
    #ifdef first_buld
        std::cout << "Edgar genius!" << std::endl;
        string l;
        while (cin >> l) {
            cout << l;
            system(("apt-get install -y " + l).c_str());
        }
    #endif
   // return 0;
    SpecCollector s;
    SpecParser p;
    Api api;
    set<string> keywords;
    keywords.insert("Obsoletes:");
    keywords.insert("Provides:");

    string branch = "p10";
    auto pnames = s.getBranchPackageNames(branch);
    int successful = 0;
   // vector<string> pnames;
    std::set<std::string> errorPackages;
   // pnames = {"opennebula","fonts-bitmap-knm-new-fixed"};
   // pnames = {"libtolua++-lua5.1", "tintin++", "tolua++", "libvsqlite++"};
    for (auto pname : pnames) {
        sleep(1);
        pname = ReplaceAll(pname, "+", "%2B");
        string spec = s.getSpec(branch, pname);
        cout << "\nIn package " << pname << ":\n";
        cerr << "\nIn package " << pname << ":\n";
        //auto packages = p.getBuildRequiresPrePackages(spec);

        auto cur_error = p.error;
        auto packages = p.getDeprecatedPackages(spec);
        if (cur_error != p.error)
            errorPackages.insert(pname);

        if (packages.size() != 0)
            successful++;
        for (auto pack : packages) {
            std::cout << pack;
            // int resp = system(("apt-get install -y " + pack).c_str()) ;
	        // if (resp != 0)
		    //     std::cout << "Error using apt-get:" << resp << std::endl;
        }
    }
    std::cout << "\nGarbage collected =) : \nnumber of packages: " << pnames.size() 
    << "\nnumber of deprecated packages: " << successful << "\nNumber of packages with error =( :" << p.error << std::endl;
    std::cout << "error packages:" << std::endl;
    for (auto name : errorPackages) {
        std::cout << name << "\n";
    }
    // std::string req = "https://rdb.altlinux.org/api/export/repology/p10";
    // req = "https://rdb.altlinux.org/api/export/branch_binary_packages/p10?arch=x86_64";
    // Json::Value list_p = Api::getReadBuffer(req); //["packages"];
    // std::cout << list_p << std::endl;
    // /*
    // for (auto p : list_p) {
    //     std::cout << "Package name source: " << p["name"] << " Binary: " << p["binaries"][0]["name"] << std::endl;
    // }
    // */
    return 0;
}