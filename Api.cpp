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
    
    // sleep на 1000 мс те на 1с
    usleep(1000 * 1000);

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
std::vector<std::string> Api::getActiveBranches() {
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

std::set<std::string> Api::getBranchPackageNames(std::string branch) {
    //std::string req = "https://rdb.altlinux.org/api/packageset/active_packagesets";
    std::string host = apiURL+"/api/export/sitemap_packages/";
    std::string req = host + branch;
    std::set<std::string> result;
    auto root = getReadBuffer(req).root;
    for (auto pack : root["packages"]) {
        result.insert(pack["name"].asString());
    }
    return result;
}

std::vector<Aux::checked_package> Api::checkPackage(std::vector<std::string> pnames, std::string branch, Cacher& ch) {
    std::vector<Aux::checked_package> out;

    // ------------------------ Кеширование TODO
    std::vector<std::string> not_cache_pnames; // собираем список пакетов, которых нет в кеше

    std::cout << "Api start " << pnames.size() <<"\n";

    for (auto pn: pnames)
    {
        auto resp = ch.getCache(pn);
        if (resp.has_value()) {
            auto val = resp.value();
            out.push_back(val);
        } else {
            not_cache_pnames.push_back(pn);
        }   
    }
    pnames = not_cache_pnames;

    // ------------------------
     
    
    std::string host = apiURL+"/api/package/what_depends_src?";
    std::string oss_test;
    if (!pnames.empty()) {
        for (int i = 0; i < pnames.size() - 1; i++) { 
            oss_test += pnames[i] + ",";
        }
        oss_test += pnames[pnames.size() - 1];
    } else {    
        return out;
    }

    if (oss_test == "") {
        return out;
    }
    std::string req = host + "packages=" + oss_test + "&branch=" + branch + "&depth=3&dptype=both&finite_package=false&oneandhalf=false&use_last_tasks=false";

    Api::response resp = getReadBuffer(req);
    long http_code = resp.http_code;
    
    

    if (http_code == 200) {
        for(auto packageName: pnames) {
            bool isFound = false; // проверяет на то что пакет нашелся у кого-то в requires
            for (auto S: resp.root["dependencies"]) {
                for (auto name: S["requires"]) {
                    if (name == packageName) {
                        // тк пакет packageName есть у кого-то в requires ставим can_delete false
                        Aux::checked_package a;
                        a.can_delete = false;
                        a.http_code = 200;
                        a.name = packageName;
                        out.push_back(a);
                        isFound = true;
                        continue;
                    }
                }
                
                if(isFound) {
                    continue;
                }
            }

            if (!isFound) {
                // тк ни у кого в requires нет такого пакета то can_delete true
                Aux::checked_package a;
                a.can_delete = true; 
                a.http_code = 200;
                a.name = packageName;
                out.push_back(a);
            }

        }
    } else {
        std::cout << "Странный ответ от апи/прокси: " << http_code << " " << oss_test << std::endl;
    }

    // 
    for (auto pack: out) {
        std::cout << "Api set cache\n";
        ch.setCache(pack.name, pack);
    } 
    //  ----------------  
    return out;
}

std::vector<Aux::checked_package> Api::divide_et_impera(std::vector<std::string> packagesNames, std::string branch, Cacher& ch) {
    // divide et impera
    // Выборка по 10 пакетов, чтобы предотваратить слишком длинные запросы (хорошо бы вынести как регулируемый параметр)
    if (packagesNames.size() > 80) {
        std::vector<Aux::checked_package> result;
        for (int i = 0; i < packagesNames.size();) {
            std::vector<std::string> pnames;
            auto index = std::min(packagesNames.size(), i + size_t(80));
            for (; i < index; i++) {
                pnames.push_back(packagesNames[i]);
            }
            std::cout << pnames.size() << std::endl;
            auto res = checkPackage(pnames, branch, ch);
            for (auto pack : res) {
                result.push_back(pack);
            }
        }
        return result;
    }

    // TODO: если пакет с данным именем присутствует в Requires api то false иначе true
    // https://rdb.altlinux.org/api/package/what_depends_src?packages=python3-dev&branch=p10&depth=5&dptype=both&finite_package=false&oneandhalf=false&use_last_tasks=false

    return checkPackage(packagesNames, branch, ch);
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

std::optional<std::string> Api::getSpecFile(std::string branch, std::string name)
{   
    std::string host = apiURL+"/api/package/specfile_by_name";
    std::string req = host + "?" + "branch=" + branch + "&" + "name=" + name;

    auto resp = getReadBuffer(req);
    if (resp.http_code != 200) {
        return std::nullopt;
    }

    std::string content = resp.root["specfile_content"].asString();
    std::string result = base64_decode(content);

    return result;
}

