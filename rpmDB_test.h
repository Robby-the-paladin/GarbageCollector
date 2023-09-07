#pragma once
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <errno.h>
#include <fcntl.h>
#include <rpm/rpmlib.h>
#include <rpm/header.h>
// #include <rpm/dbindex.h>
#include <unistd.h>
#include <rpm/rpmdb.h>

class rpmDB_test {
    public:
        rpmDB_test(){};
        std::map<std::string,std::set<std::string>> test();
        std::set<std::string> get_unic_last_name();
};