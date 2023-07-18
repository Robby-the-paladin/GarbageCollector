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
    
	Api() {};

    static bool checkPackage(std::string packageName); // проверяет, можно ли удалять пакет, возвращает true если можно
    static std::vector<std::string> getActivePackages(); // получает список активных веток
    static response getReadBuffer(std::string req); // делает запрос по url и возвращает ответ в jsonформате
};