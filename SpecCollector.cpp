#include "SpecCollector.h"

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string SpecCollector::getSpec(std::string branch, std::string name) {
    std::string host = "https://rdb.altlinux.org/api/package/specfile_by_name";
    std::string req = host + "?" + "branch=" + branch + "&" + "name=" + name;
    //std::cout << "\n" << req << "\n";

    // Запрос на получение spec к API
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, req.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        //std::cout << readBuffer << std::endl;
    }

    Json::Value root;
    Json::Reader reader;
    bool parser_success = reader.parse(readBuffer, root);
    if (!parser_success) {
        std::cout << "JSON parser error\n";
        return "";
    }

    std::string content = root["specfile_content"].asString();
    std::string result = base64_decode(content);

    //std::cout << "result:\n" << result;

    return result;
}