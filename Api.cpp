#include "Api.h"


static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
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
        curl_easy_setopt(curl, CURLOPT_URL, req.c_str());
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer); 
        res = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        // if (http_code != 200)
        //     std::cout << http_code  << "\n";
        
        curl_easy_cleanup(curl);
    }

    Json::Value root;
    Json::Reader reader;
    bool parser_success = reader.parse(readBuffer, root);
    if (!parser_success) {
        std::cout << "JSON parser error\n";
        return {NULL, http_code};
    }
    return {root, http_code};
}

std::vector<std::string> Api::getActivePackages() {
    std::string req = "https://rdb.altlinux.org/api/packageset/active_packagesets";
    Json::Value root = getReadBuffer(req).root;
    
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
    int index = 1;
    for (auto packageset : packagesets) {
        std::cout << packageset << " " <<  index << "/" << packagesets.size() << std::endl;
        std::string req = host + "branch=" + packageset + "&dp_name=" + packageName + "&dp_type=require";
        sleep(1);
        long http_code = getReadBuffer(req).http_code;
        if (http_code == 200) {
            return false;
        }
        index++;
    }

    return true;
}