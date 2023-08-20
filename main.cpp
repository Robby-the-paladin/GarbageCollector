#include <iostream>
#include <regex>
#include <unistd.h>
#include <set>
#include <map>
#include "SpecCollector.h"
#include "SpecParser.h"
#include "PostgreHandler.h"
#include "Api.h"
//#include <pqxx/pqxx>

using namespace std;

//#define first_buld
//#define insert_data

std::ostream& operator << (std::ostream &os, const SpecParser::lib_data &lib)
{
    return os << lib.name << " " << lib.sign << " " << lib.version << " " << lib.type;
}

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

std::vector<std::string> Api::activePackages = Api::getActivePackages();

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
    PostgreHandler ph;
    Api api;
    set<string> keywords;
    keywords.insert("Obsoletes:");
    keywords.insert("Provides:");
    auto save_names = ph.getAllNames();
    //return 0;
    string branch = "p10";
    vector<string> pnames = s.getBranchPackageNames(branch);
    int successful = 0;
   // vector<string> pnames;
    std::set<std::string> errorPackages;
    
    // pnames = {"opennebula","fonts-bitmap-knm-new-fixed"};
    // pnames = {"boost"};
    //pnames = {"libtolua++-lua5.1", "tintin++", "tolua++", "libvsqlite++"};
    #ifdef insert_data
        for (auto pname : pnames) {
            pname = ReplaceAll(pname, "+", "%2B");

            sleep(1);
            string spec = s.getSpec(branch, pname);
            cout << "\nIn package " << pname << ":\n";
            cerr << "\nIn package " << pname << ":\n";
            //auto packages = p.getBuildRequiresPrePackages(spec);

            auto cur_error = p.error;
            auto packages = p.getDeprecatedPackages_test(spec);

            auto test = p.strToStructSet_lib_data(packages);

            if (cur_error != p.error) {
            errorPackages.insert(pname);
            } else {
                ph.addDeprecated(pname, "data", packages);
            }

            for (auto pack : test) {
            std::cout << pack << " ; ";
            }
            std::cout << '\n';
        }
        return 0;
    #endif
    /*
    for (auto pname : pnames) {
        

        pname = ReplaceAll(pname, "+", "%2B");
        if (save_names.find(pname) != save_names.end()) {
            cout << "SKIP package name: " << pname << endl;
            continue;
        }
        
        sleep(1); 
    for (auto pname : pnames) {
        

        pname = ReplaceAll(pname, "+", "%2B");
        if (save_names.find(pname) != save_names.end()) {
            cout << "SKIP package name: " << pname << endl;
            continue;
        }
        
        sleep(1);
        string spec = s.getSpec(branch, pname);
        cout << "\nIn package " << pname << ":\n";
        cerr << "\nIn package " << pname << ":\n";
        //auto packages = p.getBuildRequiresPrePackages(spec);

        auto cur_error = p.error;
        auto packages = p.getDeprecatedPackages(spec);
        if (cur_error != p.error) {
           errorPackages.insert(pname);
        } else {
            ph.addDeprecated(pname, "data", packages);
        }
        
        if (packages.size() != 0)
            successful++;
       
        for (auto pack : packages) {
           std::cout << pack;
        }
        
    }
        string spec = s.getSpec(branch, pname);
        cout << "\nIn package " << pname << ":\n";
        cerr << "\nIn package " << pname << ":\n";
        //auto packages = p.getBuildRequiresPrePackages(spec);

        auto cur_error = p.error;
        auto packages = p.getDeprecatedPackages(spec);
        if (cur_error != p.error) {
           errorPackages.insert(pname);
        } else {
            ph.addDeprecated(pname, "data", packages);
        }
        
        if (packages.size() != 0)
            successful++;
       
        for (auto pack : packages) {
           std::cout << pack;
        }
        
    }*/
    
    
    
    // std::cout << "\nGarbage collected =) : \nnumber of packages: " << pnames.size() 
    // << "\nnumber of deprecated packages: " << successful << "\nNumber of packages with error " << ((p.error)? "=( :" : "=) :") << p.error << std::endl;
    // std::cout << "error packages:" << std::endl;
    // for (auto name : errorPackages) {
    //    std::cout << name << "\n";
    // }
    
    
    //map<string, bool> checked_packages = {};
    cout << "\nSearching deprecated packages\n";
    for (auto pname : pnames) {
        pname = ReplaceAll(pname, "+", "%2B");
        
        // if (!ph.isDeprecatedNull(pname)) {
        //     cout << "SKIP package name: " << pname << endl;
        //     continue;
        // }
        cout << "\nIn package " << pname << ":\n";
        cerr << "\nIn package " << pname << ":\n";
        set<string> data, depr_data;
        ph.getDeprecated(pname, "data", data);

        vector<SpecParser::lib_data> structs_data = SpecParser::strToStructSet_lib_data(data);

        for (auto pack : structs_data) {
            
            pack.name = ReplaceAll(pack.name, "+", "%2B");
            pack.name = ReplaceAll(pack.name, " ", "");
            // if (checked_packages.find( pack ) != checked_packages.end()) {
            //     if (pack != "" && checked_packages[pack]) {
            //         depr_data.insert(pack);
            //         cout << "SKIP HASHED pack name: " << pack << endl;
            //     }
            //     continue;
            // }
            sleep(1);
            cout << "   " << pack.name << endl;
            if (pack.name != "" && ph.getCheckedPackage(pack.name)) {
                //cout << pack << " ";
                depr_data.insert(pack.name);
            }
        }
        cout << "\n";
        ph.addDeprecated(pname, "depr_data", depr_data);
    }
    
    //ph.connect.disconnect();

    return 0;
}