#include "Api.h"


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
    std::string req = "http://64.226.73.174:8080/api/packageset/active_packagesets";
    Json::Value root = getReadBuffer(req).root;
    
    Json::Value name = root["packagesets"];
    std::vector<std::string> packagesets;

    for (auto branch : name) {
        packagesets.push_back(branch.asString());
    }
    return packagesets;
}

Api::checked_package Api::checkPackage(std::string packageName, std::string branch) {
    // TODO: если пакет с данным именем присутствует в Requires api то false иначе true
    std::string host = "https://rdb.altlinux.org/api/dependencies/packages_by_dependency?";

    // https://rdb.altlinux.org/api/package/what_depends_src?packages=python3-dev&branch=p10&depth=5&dptype=both&finite_package=false&oneandhalf=false&use_last_tasks=false

    //std::vector<std::string> packagesets = getActivePackages();

    host = "http://64.226.73.174:8080/api/package/what_depends_src?";

    std::string req = host + "packages=" + packageName + "&branch=" + branch + "&depth=5&dptype=both&finite_package=false&oneandhalf=false&use_last_tasks=false";

    Api::response resp = getReadBuffer(req);
    long http_code = resp.http_code;

    if (http_code == 200) {
        for (auto S: resp.root["dependencies"]) {
            for (auto name: S["requires"]) {
                if (name == packageName) {
                    checked_package a;
                    a.can_delete = false;
                    a.http_code = 200;
                    return a;
                }
            }
        }
    } else {
        checked_package a;
        a.can_delete = false;
        a.http_code = http_code;
        return a;
    }
    checked_package a;
    a.can_delete = true;
    a.http_code = 404;
    return a;


    /*
    int index = 1;
    for (auto packageset : activePackages) {
        std::cout << packageset << " " <<  index << "/" << activePackages.size() << std::endl;
        std::string req = host + "branch=" + packageset + "&dp_name=" + packageName + "&dp_type=require";
        sleep(1);
        long http_code = getReadBuffer(req).http_code;
        if (http_code == 200) {
            checked_package a;
            a.can_delete = false;
            a.http_code = 200;
            return a;
        } else if (http_code != 200 && http_code != 404) {
            checked_package a;
            a.can_delete = false;
            a.http_code = http_code;
            return a;
        }
        index++;
    }
    
    checked_package a;
    a.can_delete = true;
    a.http_code = 404;
    return a;
    */
}