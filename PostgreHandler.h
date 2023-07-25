#pragma once
#include <iostream>
#include <string>
#include <set>
#include <unistd.h>
#include <json/json.h>
#include <vector>
#include <pqxx/pqxx>
#include "Api.h"

class PostgreHandler {
public:
    pqxx::connection connect;
	PostgreHandler();
    void reconnect();
    
    bool addDeprecated(std::string name, std::string col, std::set<std::string> data); 
    bool getDeprecated(std::string name, std::string col, std::set<std::string>& data);

    bool isDeprecatedNull(std::string name);

    bool getCheckedPackage(std::string name);
    bool setCheckedPackage(std::string name);

    std::set<std::string> getAllNames();
    bool replaceDeprecatedWith(std::string name, std::string col, std::set<std::string> data);
};


