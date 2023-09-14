#include "Api.h"

extern std::string apiURL;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

Api::response Api::getReadBuffer(std::string req) {
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    long http_code = 0;
    curl = curl_easy_init();
    if (curl) {
        //std::cout << req << "\n";
        curl_easy_setopt(curl, CURLOPT_URL, req.c_str());
        //таймаут для запроса (иначе треш)
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 305);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code != 200)
            std::cout << http_code  << "\n";        
        
        curl_easy_cleanup(curl);
    }

    Json::Value root;
    Json::Reader reader;
    bool parser_success = reader.parse(readBuffer, root);
    //std::cout << readBuffer << "\n";
    if (!parser_success) {
        std::cout << "JSON parser error\n";
        return {root, http_code};
    }
    return {root, http_code};
}

/// @brief 
/// @return 
std::vector<std::string> Api::getActivePackages() {
    //std::string req = "https://rdb.altlinux.org/api/packageset/active_packagesets";
    std::string req = apiURL+"/api/packageset/active_packagesets";
    Json::Value root = getReadBuffer(req).root;
    
    Json::Value name = root["packagesets"];
    std::vector<std::string> packagesets;

    for (auto branch : name) {
        packagesets.push_back(branch.asString());
    }
    return packagesets;
}

std::vector<Api::checked_package> Api::checkPackage(std::vector<std::string> packagesNames, std::string branch) {
    // divide et impera
    // Выборка по 10 пакетов, чтобы предотваратить слишком длинные запросы (хорошо бы вынести как регулируемый параметр)
    if (packagesNames.size() > 10) {
        std::vector<Api::checked_package> result;
        for (int i = 0; i < packagesNames.size();) {
            std::vector<std::string> pnames;
            for (; i < std::min(packagesNames.size(), i + size_t(10)); i++) {
                pnames.push_back(packagesNames[i]);
            }
            auto res = checkPackage(pnames, branch);
            for (auto pack : res) {
                result.push_back(pack);
            }
        }
        return result;
    }

    // TODO: если пакет с данным именем присутствует в Requires api то false иначе true


    // https://rdb.altlinux.org/api/package/what_depends_src?packages=python3-dev&branch=p10&depth=5&dptype=both&finite_package=false&oneandhalf=false&use_last_tasks=false

    //std::vector<std::string> packagesets = getActivePackages();
    std::string host = apiURL+"/api/package/what_depends_src?";

    std::ostringstream oss;
    std::vector<Api::checked_package> out;
    if (!packagesNames.empty()) {
        std::copy(packagesNames.begin(), packagesNames.end() - 1, std::ostream_iterator<std::string>(oss, ","));
        oss << packagesNames.back();
    } else {
        return out;
    }
   // std::cout << "Проверяются пакеты: " << oss.str() << " !!!!!!!!!!!!!!!"<< std::endl;
    if (oss.str() == "") {
        return out;
    }
    std::string req = host + "packages=" + oss.str() + "&branch=" + branch + "&depth=3&dptype=both&finite_package=false&oneandhalf=false&use_last_tasks=false";

    std::cout << packagesNames.size() << " Проверяются пакеты: " << req << " !!!!!!!!!!!!!!!"<< std::endl;

    Api::response resp = getReadBuffer(req);
    long http_code = resp.http_code;
    
    

    if (http_code == 200) {
        for(auto packageName: packagesNames) {
            bool can_delete = false;
            for (auto S: resp.root["dependencies"]) {
                for (auto name: S["requires"]) {
                    if (name == packageName) {
                        checked_package a;
                        a.can_delete = false;
                        a.http_code = 200;
                        a.name = packageName;
                        out.push_back(a);
                        can_delete = true;
                        continue;
                    }
                }
                if(can_delete) {
                    continue;
                }
            }
            if (!can_delete) {
                checked_package a;
                a.can_delete = true;
                a.http_code = 404;
                a.name = packageName;
                out.push_back(a);
            }

        }
    } else {
        std::cout << "Странный ответ от апи/прокси: " << http_code << " " << oss.str() << std::endl;
    }
    return out;
}

std::optional<std::string> Api::getHash(std::string branch, std::string name){
    auto result = getReadBuffer(apiURL + "/site/pkghash_by_name?branch=" + branch + "&name=" + name);
    if (result.http_code == 404) {
        result = getReadBuffer(apiURL + "pkghash_by_binary_name?branch=" + branch + "&name=" + name + "&arch=x86_64");
        if (result.http_code == 404) {
            result = getReadBuffer(apiURL + "pkghash_by_binary_name?branch=" + branch + "&name=" + name + "&arch=noarch");
        }
    }
    if (result.http_code != 200)
        return std::nullopt;
    return result.root["pkghash"].asString();
}

std::optional<std::pair<long long, std::string>> Api::getDate(std::string branch, std::string name) {
    auto hash_ = getHash(branch, name);
    if (!hash_.has_value())
        return std::nullopt;
    std::string hash = *hash_;
    auto result = getReadBuffer(apiURL + "site/package_info/" + hash + "?branch=" + branch + "&changelog_last=1&package_type=source");
    if (result.http_code == 404) {
        result = getReadBuffer(apiURL + "site/package_info/" + hash + "?branch=" + branch + "&changelog_last=1&package_type=binary");
    }
    if (result.http_code != 200)
        return std::nullopt;
    std::pair<int, std::string> date = {result.root["buildtime"].asInt64(), result.root["task_date"].asString()};
    return date;
}
