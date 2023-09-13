#pragma once
#include <iostream>
#include <string>
#include <set>
#include <unistd.h>
#include <mutex>
#include <json/json.h>
#include <chrono>
#include <vector>
#include <pqxx/pqxx>
#include "Api.h"

class PostgreHandler {
public:
    std::mutex ph_lock;
    int test = 0;
    pqxx::connection connect;
	PostgreHandler();
    void reconnect();
    
    bool addDeprecated(std::string name, std::string col, std::set<std::string> data); 
    bool getDeprecated(std::string name, std::string col, std::set<std::string>& data);
    bool addCount(std::string name, int count);

    bool isDeprecatedNull(std::string name);

    std::vector<std::string> getCheckedPackages(std::vector<std::string> names, std::string branch, std::set<std::string>& unic_list);
    bool setCheckedPackage(std::string name);

    std::set<std::string> getAllNames();
    std::set<std::string> getNamesWithData();
    bool replaceDeprecatedWith(std::string name, std::string col, std::set<std::string> data);

    bool checkDeprDate(std::string pname, std::string branch);
    void remove_provides(std::string key, std::string remove_name);
};


