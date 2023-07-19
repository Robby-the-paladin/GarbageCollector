#pragma once
#include <iostream>
#include <string>
#include <set>
#include <unistd.h>
#include <json/json.h>
#include <vector>
#include <pqxx/pqxx>

class PostgreHandler {
public:
    pqxx::connection connect;
	PostgreHandler();
    
    bool addDeprecated(std::string name, std::string col, std::set<std::string> data); 
    bool getDeprecated(std::string name, std::string col, std::set<std::string>& data);

    bool isDeprecatedNull(std::string name);

    std::set<std::string> getAllNames();
};


