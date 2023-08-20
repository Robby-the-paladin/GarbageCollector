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

    static checked_package checkPackage(std::string packageName, std::string branch); // проверяет, можно ли удалять пакет, возвращает true если можно
    static std::vector<std::string> getActivePackages(); // получает список активных веток
    static response getReadBuffer(std::string req); // делает запрос по url и возвращает ответ в jsonформате

    static std::vector<std::string> activePackages;
};