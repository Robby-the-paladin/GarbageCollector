#include <iostream>
#include <unistd.h>
#include <set>
#include <regex>
#include <map>
#include "Api.h"
#include "LegacyDependencyAnalyzer.h"
#include "PatchMaker.h"
#include "Cacher.h"
#include "Config.h"
#include <htmlcxx/html/ParserDom.h>
#include <curl/curl.h>

using namespace std;
using namespace htmlcxx;

// А точно безоавсно хранить админский ключ в строке на гите, и так сойдет ...
std::string postgreConnStr = "";
//Прокся
//std::string apiURL = "http://64.226.73.174:8080";
//не прокся (медленно)
std::string apiURL = "https://rdb.altlinux.org";

// возвращает true если пакет подходит хотя бы под одно регулярное выражение из конфига
bool packageExamined(std::string pack, const std::vector<std::string>& pack_regexs, const vector<string>& list_errors) {

    for (auto r: pack_regexs) {
        std::regex R(r);
        std::smatch cm;
        if (std::regex_search(pack, cm, R)) {
            for (auto err: list_errors) {
                std::regex R(pack + string(".*"));
                std::smatch cm;
                if (std::regex_search(err, cm, R)) {
                    cout << "WARNING! " << pack << endl;
                }
            }

            return true;
        }
    }
    return false;
}

bool ask_user(std::vector<std::string> ignore_list){
    char choice;

    std::cout << "WARNING: Some packages from the list for processing is not built in the current release:\n";
    for (int i = 0; i < ignore_list.size(); i++) {
        cout << "\t-\t" << ignore_list[i] << "\n";
    }
    cout << "To process them run with -e or --enable-processing-error-pkg\n";
    cout << "Proceed without these packages? [Y/n]:";
    choice = getchar();

    if (choice == 'N' || choice == 'n') {
        return false;
    }
    std::cout << "Continuing..." << std::endl;
    return true;
}

std::set<std::string> check_error(std::set<std::string> packs, std::vector<std::string> error_list) {
    std::vector<std::string> err_l; 
    std::set<std::string> not_err;
    for (auto pack: packs) {
        for (auto err: error_list) {
            std::regex R(pack + string(".*"));
            std::smatch cm;
            if (std::regex_search(err, cm, R)) {
                err_l.push_back(pack);
            } else {
                not_err.insert(pack);
            }
        }   
    }

    if (ask_user(err_l)) {
        return not_err;
    }
    return packs;
}

size_t WriteCallback2(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

vector<string> get_list_errors() {
    string req = "http://git.altlinux.org/beehive/logs/Sisyphus-x86_64/latest/error/";
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
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback2);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code != 200)
            std::cout << http_code  << "\n";        
        
        curl_easy_cleanup(curl);
    }

    string html_content = readBuffer;
    // Parse the HTML content
    HTML::ParserDom parser;
    tree<HTML::Node> dom = parser.parseTree(html_content);

    // Find all 'a' tags with class 'link' and extract the 'href' attribute
    tree<HTML::Node>::iterator it = dom.begin();
    tree<HTML::Node>::iterator end = dom.end();
    ++it;
    vector<string> list_errors;
    for (; it != end; ++it) {
        if (it->tagName() == "a") {
            it->parseAttributes();
            string href = it->attribute("href").second;
            // cout << href << endl;
            list_errors.push_back(href);
        }
    }
    return list_errors;
}

// Исключать пакеты из списка не собираемых пакетов
bool ignore_error_pkgs = true;

// Обработчик флагов запуска
bool process_flags(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (string(argv[i]) == "-e" || string(argv[i]) == "--enable-processing-error-pkg") {
            ignore_error_pkgs = false;
            continue;
        }
        if (string(argv[i]) == "--help") {
            cout << "-e --enable-processing-error-pkg   Processing packages from error list\n";
            cout << "--help                             Printing this text\n";
            return false;
        }
        cout << "Unrecognised flag: " << argv[i] << "\n";
        cout << "Run with --help to show variants\n";
        return false;
    }
    return true;
}

int main(int argc, char *argv[]) {
    if (!process_flags(argc, argv))
        return 0;
    Aux::init(); // инициализация глобальных констант

    auto list_error = get_list_errors();
    if (ignore_error_pkgs)
        list_error.clear();
    // return 0;
    Config cf;
    postgreConnStr = cf.getConnectDB();
    auto pack_regexs = cf.getPackRegexs();

    std::cout << postgreConnStr << std::endl;
    Cacher CH;

    auto  L = LegacyDependencyAnalyzer();
    
    auto t = Api::getBranchPackageNames("sisyphus");
    set<std::string> test;
    int index = 0;
    for (auto it = t.begin(); it != t.end(); it++) {
        if (*it == "boost") {
            std::cout << "boost " << packageExamined(*it, pack_regexs, list_error) << "\n";
        }
        if (!packageExamined(*it, pack_regexs, list_error)) {
            continue;
        }

        test.insert(*it);
        index++;
    }
   
    test = check_error(test, list_error);

    std::cout << "analysingBranchPackages   " << test.size() << " " << *(test.begin()) << std::endl;
    L.analysingBranchPackages(test);
 

    std::vector<std::string> packages;
    std::map<std::string, std::pair<std::string, std::string>> test_pack;
    std::cout << "packagesToAnalyse " << L.packagesToAnalyse.size() << std::endl;
    for(auto pack: L.packagesToAnalyse) {
        cout << pack.second.first << endl;
        packages.push_back(pack.second.first);
        test_pack[pack.first] = pack.second;
        // if (count <= 0) {
        //     L.packagesToAnalyse = test_pack;
        //     break;
        // }
        // count--;
    }

    L.packagesToAnalyse = test_pack;
    std::cout << L.packagesToAnalyse.size() << std::endl;

    auto P = PatchMaker();
    P.dependenciesToDelete = L.criteriaChecking(CH);
    P.packagesToPatch = L.packagesToFix;
    cout << "L.pakagesToFix.size(): " << L.packagesToFix.size();
    for (int i = 0; i < L.packagesToFix.size(); i++) {
        cout << L.packagesToFix[i] << std::endl;
    }
    P.loadSpecs(PatchMaker::specLoader::apiLoader);
    P.makePatch("./Patches3_TEST/");

    return 0;
}
