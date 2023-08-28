#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <sstream>
#include <curl/curl.h>
#include <json/json.h>
#include <unistd.h>
#include "base64.h"

class Api {
public:
    struct response
    {
        Json::Value root;
        long http_code;
    };
    
    struct checked_package {
        std::string name;
        long http_code;
        bool can_delete;
    };
    

	Api() {};

    std::vector<checked_package> checkPackage(std::vector<std::string> packagesNames, std::string branch); // проверяет, можно ли удалять пакет, возвращает true если можно
    std::vector<std::string> getActivePackages(); // получает список активных веток
    response getReadBuffer(std::string req); // делает запрос по url и возвращает ответ в jsonформате

    std::optional<std::string> getHash(std::string branch, std::string name);
    std::optional<std::pair<long long, std::string>> getDate(std::string branch, std::string name);

    std::vector<std::string> activePackages;
};