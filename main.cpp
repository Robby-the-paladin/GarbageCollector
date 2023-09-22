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
//#include <pqxx/pqxx>

using namespace std;

// А точно безоавсно хранить админский ключ в строке на гите, и так сойдет ...
std::string postgreConnStr = "user=doadmin password=AVNS_xMD70wwF41Btbfo6iaz host=db-postgresql-fra1-79796-do-user-14432859-0.b.db.ondigitalocean.com port=25060 dbname=test target_session_attrs=read-write";
//std::string postgreConnStr = "user=edgar password=genius host=host.docker.internal port=5432 dbname=test target_session_attrs=read-write";
//Прокся
std::string apiURL = "http://64.226.73.174:8080";
//не прокся (медленно)
//std::string apiURL = "https://rdb.altlinux.org";
int threadsSize = 100;

std::set<std::string> errorPackages;

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

PostgreHandler ph;

std::set<std::string> unic_list;


void parseData(std::string pname, std::string branch) {
    //std::cout << "Parsing " << pname << " ..." << std::endl;
    SpecCollector s;
    SpecParser p;
    pname = ReplaceAll(pname, "+", "%2B");
    //cout << "Getting spec " + pname << endl;
    string spec = s.getSpec(branch, pname);
    cout << "\nIn package " << pname << ":\n";
    cerr << "\nIn package " << pname << ":\n";
            //auto packages = p.getBuildRequiresPrePackages(spec);

    auto cur_error = p.error;

    ph.ph_lock.lock();
    auto packages = p.getDeprecatedPackages_test(spec);
    ph.ph_lock.unlock();
    
    auto test = p.strToStructSet_lib_data(packages);

    if (cur_error != p.error) {
        errorPackages.insert(pname);
    } else {
        ph.ph_lock.lock();
        ph.addDeprecated(pname, "data", packages);
        ph.ph_lock.unlock();
    }

    for (auto pack : test) {
        std::cout << pack << " ; ";
    }
    std::cout << '\n';
}

void deprCheck(std::string pname, std::string branch) {
    std::cout << "\nSearching deprecated packages\n" << ph.test << " " << std::endl;
    ph.test = ph.test + 1;
    pname = ReplaceAll(pname, "+", "%2B");
    
    // if (!ph.isDeprecatedNull(pname)) {
    //     cout << "SKIP package name: " << pname << endl;
    //     continue;
    // }
    std::cout << "\nIn package " << pname << ":\n";
    cerr << "\nIn package " << pname << ":\n";
    set<string> data, depr_data;

    ph.ph_lock.lock();
    ph.getDeprecated(pname, "data", data);
    ph.ph_lock.unlock();

    vector<SpecParser::lib_data> structs_data = SpecParser::strToStructSet_lib_data(data);
    int index = 0;
    vector<std::string> names;
    for (auto pack : structs_data) {
        if (pack.name == "") {
            continue;
        }
        pack.name = ReplaceAll(pack.name, "+", "%2B");
        pack.name = ReplaceAll(pack.name, " ", "");
        pack.name = ReplaceAll(pack.name, "    ", "");
       // sleep(1);
        
        names.push_back(pack.name);
      //  std::cout << "   " << pack.name << endl;
        index++;
    }

    std::vector<std::string> can_delete = ph.getCheckedPackages(names, branch, unic_list);
    depr_data = std::set<std::string>(can_delete.begin(), can_delete.end());
    
    ph.ph_lock.lock();
    ph.addDeprecated(pname, "depr_data", depr_data);
    ph.ph_lock.unlock();

}

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
    string branch = "sisyphus";
    std::map<string, bool> actionsMap;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-b") != 0) {
            branch = argv[++i];
        } else {
            actionsMap[argv[i]] = true;
        }
    }

    cout << "Ветка проверки: " << branch << endl;

    if (argc == 1){
        cout<<"Передайте в командной строке что вы хотите сделать"<<endl;
        cout<<"first_buld, insert_data, insert_depr_data"<<endl;

        return 0;
    }
    int maxThreads = std::thread::hardware_concurrency() ;
    if (maxThreads < threadsSize) {
        threadsSize = maxThreads;
    }
     if (actionsMap.find("first_buld") != actionsMap.end()){
        string l;
        while (cin >> l) {
            cout << l;
            system(("apt-get install -y " + l).c_str());
        }
    }
    try {
        cout<<"connect to db";
    // Connect to the database
        pqxx::connection c(postgreConnStr);
    } catch (const exception &e) {
        cerr << e.what() << endl;
    }
   
    SpecCollector s;
    SpecParser p;
    PostgreHandler ph;
    Api api;
    set<string> keywords;
    keywords.insert("Obsoletes:");
    keywords.insert("Provides:");
    //return 0;
    
    cout << "Branch: " << branch << endl;
    vector<string> pnames = s.getBranchPackageNames(branch);
    auto unicalPackages = getUnicalPackageNames(pnames,ph.getNamesWithData());
    cout << "Packages: " << pnames.size() << " Unical "<< unicalPackages.size()<< endl;
    int successful = 0;
   // vector<string> pnames;
    int system_threads_count_insert_data = 0;
    int system_threads_count_insert_depr_data = 0;
    
    // pnames = {"opennebula","fonts-bitmap-knm-new-fixed"};
    // pnames = {"boost"};
    //pnames = {"libtolua++-lua5.1", "tintin++", "tolua++", "libvsqlite++"};

    if (actionsMap.find("insert_data") != actionsMap.end()){
        rpmDB_test r;
        std::map<std::string,std::set<std::string>> packages = r.test();
        
        PostgreHandler phh;
        auto saves = ph.getNamesWithData();
        int count = packages.size();
        int index = 0;
        for(auto elem: packages) {
            std::cout << elem.first << ": ";
            if (saves.find(elem.first) == saves.end()) {
                phh.addDeprecated(elem.first, "data", elem.second);
                phh.addCount(elem.first, elem.second.size());
            } else {
                std::cout << "SKIP: ";
            }
            std::cout << index++ << " / " << count << std::endl;
        }
    }

    rpmDB_test r;
    unic_list = r.get_unic_last_name();

    auto save_names = ph.getAllNames();
    save_names = ph.getNamesWithData();
    if (actionsMap.find("insert_depr_data") != actionsMap.end()){
        std::queue<std::thread> threads;
        for (auto pname : save_names) {
            std::thread thr(deprCheck, pname, branch);
            threads.push(std::move(thr));
            while(threads.size() > threadsSize) {
                threads.front().join();
                threads.pop();
            }
        }
        while (!threads.empty()) {
            threads.front().join();
            threads.pop();
        }
    }

    if (actionsMap.find("remove_provides") != actionsMap.end() || true){
        cout << "REMOVE" << endl;
        r.remove_provides(ph);
    }

    return 0;
}
