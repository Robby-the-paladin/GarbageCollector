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
    struct response
    {
        Json::Value root;
        long http_code;
    };
    
    struct checked_package {
        long http_code;
        bool can_delete;
    };
    

	Api() {};

    checked_package checkPackage(std::string packageName, std::string branch); // проверяет, можно ли удалять пакет, возвращает true если можно
    std::vector<std::string> getActivePackages(); // получает список активных веток
    response getReadBuffer(std::string req); // делает запрос по url и возвращает ответ в jsonформате

    std::vector<std::string> activePackages;
};