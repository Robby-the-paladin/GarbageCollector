#include "SpecCollector.h"

extern std::string apiURL;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

Api::response SpecCollector::getSpecResponse(std::string branch, std::string name) {
    Api a;
    std::string host = apiURL+"/api/package/specfile_by_name";
    std::string req = host + "?" + "branch=" + branch + "&" + "name=" + name;
    auto root = a.getReadBuffer(req);

    return root;
}

std::vector<std::string> SpecCollector::getBranchPackageNames(std::string branch) {
    std::vector<std::string> result;
    Api a;
    std::string host = apiURL+"/api/export/sitemap_packages/";
    std::string req = host + branch;
    std::cout << req << std::endl;
    auto root = a.getReadBuffer(req).root;
    for (auto pack : root["packages"]) {
        result.push_back(pack["name"].asString());
    }

    return result;
}

std::string SpecCollector::getSpec(std::string branch, std::string name) {
    auto resp = getSpecResponse(branch, name);
    if (resp.http_code != 200) {
        return "";
    }
    std::string content = resp.root["specfile_content"].asString();
    std::string result = base64_decode(content);
    return result;
}

std::string SpecCollector::getSpecDate(std::string branch, std::string name) {
    auto resp = getSpecResponse(branch, name);
    if (resp.http_code != 200) {
        return "";
    }
    std::string result = resp.root["specfile_date"].asString();
    return result;
}