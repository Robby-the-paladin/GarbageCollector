#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <json/json.h>
#include <unistd.h>
#include "base64.h"

class Api {
public:
	Api() {};

    bool checkPackage(std::string packageName);
    std::vector<std::string> getActivePackages();
    Json::Value getReadBuffer(std::string req);
};