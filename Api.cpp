#include "Api.h"


static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

Json::Value Api::getReadBuffer(std::string req) {
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    long http_code = 0;
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, req.c_str());
        
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
    if (!parser_success) {
        std::cout << "JSON parser error\n";
        return {};
    }
    return root;
}

std::vector<std::string> Api::getActivePackages() {
    std::string req = "https://rdb.altlinux.org/api/packageset/active_packagesets";
    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, req.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    Json::Value root;
    Json::Reader reader;
    bool parser_success = reader.parse(readBuffer, root);
    if (!parser_success) {
        std::cout << "JSON parser error\n";
        return {};
    }

    Json::Value name = root["packagesets"];
    std::vector<std::string> packagesets;

    for (auto branch : name) {
        packagesets.push_back(branch.asString());
    }
    return packagesets;
}

bool Api::checkPackage(std::string packageName) {
    // TODO: если пакет с данным именем присутствует в Requires api то false иначе true
    std::string host = "https://rdb.altlinux.org/api/dependencies/packages_by_dependency?";
    
    std::vector<std::string> packagesets = getActivePackages();

    packageName = "boost-atomic-devel";

    for (auto packageset : packagesets) {
        std::string req = host + "branch=" + packageset + "&dp_name=" + packageName + "&dp_type=require";
        
        Json::Value root = this->getReadBuffer(req);
        //usleep(500000); // 0.01 sec
        //sleep(1);
    }

    return true;
}