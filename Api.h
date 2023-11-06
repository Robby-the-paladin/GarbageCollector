#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <iterator>
#include <sstream>
#include <curl/curl.h>
#include <json/json.h>
#include <unistd.h>
#include "base64.h"

class Api {
public:
    /// структура ответа сервера
    // root - тело ответа
    // http_code - код ответа
    struct response
    {
        // тело ответа
        Json::Value root;
        // код ответа
        long http_code;
    };
    
    // информация о пакете
    // name - название пакета
    // http_code - код ответа севрера
    // can_delete - true если никем не используется, иначе false
    struct checked_package {
        // название пакета
        std::string name;
        // код ответа севрера
        long http_code;
        // true если никем не используется, иначе false
        bool can_delete;
    };
    

	Api() {};

    // проверяет, можно ли удалять пакет, возвращает вектор структур checked_package
    std::vector<checked_package> static divide_et_impera(std::vector<std::string> packagesNames, std::string branch);
    
    // получает список активных веток
    std::vector<std::string> getActiveBranches(); 

    // получение пакетов из ветки
    std::set<std::string> static getBranchPackageNames(std::string branch);

    // делает запрос по url и возвращает ответ в jsonформате
    response static getReadBuffer(std::string req); 

    std::optional<std::string> getHash(std::string branch, std::string name);
    std::optional<std::pair<long long, std::string>> getDate(std::string branch, std::string name);

    std::vector<std::string> activePackages;
    
    // получение spect файла в формате string по branch и name package
    std::optional<std::string> static getSpecFile(std::string branch, std::string name);

    private:
    // проверяет список пакетов на бесполезность. возвращает вектор структур checked_package
    std::vector<checked_package> static checkPackage(std::vector<std::string> packagesNames, std::string branch);

};