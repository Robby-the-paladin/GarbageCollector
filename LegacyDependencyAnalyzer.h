#pragma once

#include <iostream>
#include <regex>
#include <unistd.h>
#include <set>
#include <map>
#include <queue>
#include <thread>
#include <mutex>
#include "RpmHandler.h"




class LegacyDependencyAnalyzer {
    public:        
        // А точно безоавсно хранить админский ключ в строке на гите, и так сойдет ...
        // Информация для соединения с БД postgresql
        std::string postgreConnStr = "user=doadmin password=AVNS_xMD70wwF41Btbfo6iaz host=db-postgresql-fra1-79796-do-user-14432859-0.b.db.ondigitalocean.com port=25060 dbname=test target_session_attrs=read-write";
        //std::string postgreConnStr = "user=edgar password=genius host=host.docker.internal port=5432 dbname=test target_session_attrs=read-write";
        
        // Прокся 
        std::string apiURL = "http://64.226.73.174:8080";

        //не прокся (медленно) 
        //std::string apiURL = "https://rdb.altlinux.org";

        // список старых веток для анализа
        std::vector<std::string> oldBranches = {"4.0", "4.1", "p5", "p6", "p7", "p8"};

        // статичная часть названия классик файла pkglist.classic(архитектура x86_64)  
        const std::string folderClassicFiles = "oldBranches";
        std::string constNameClassic = "pkglist.classic."; 
        std::vector<std::string> classicArches = {"x86_64", "noarch"};

        // список пакетов для анализа
        std::vector<std::string> packagesToAnalyse;

        LegacyDependencyAnalyzer(){};

        // устанавливает список пакетов для анализа из ветки. По умолчанию sisyphus
        void analysingBranchPackages(std::string branch = "Sisyphus");

        // возвращает список всех зависимостей для packagesToAnalyse
        std::vector<PackageDependencies> getAllDependencies();

        // проверяет зависимости из unicDependecies на критерии 
        void criteriaChecking();
    private:
        // список уникальных зависимостей генерируемых методом getAllDependencies
        std::set<Dependency> unicDependecies;

        //
        std::set<std::string> oldPackagesNames;

        // возвращает множество пакетов, которые есть в oldBranches и отсутствуют в проверяемой ветке
        std::set<std::string> getOldPackagesNames();

        // проверка на то что в актуальной ветке нет пакетов, которые зависят от исследуемого
        // true если есть зависимость, false - иначе
        bool isAnythingDependsSrc(std::string packageName);

        // проверка на то что в актуальной ветке нет пакетов, которые зависят от исследуемых и возвращает 
        // ответ для каждого пакета. 
        // true если есть зависимость, false - иначе
        std::map<std::string, bool> isAnythingDependsSrc(std::vector<std::string> packagesNames);

        // проверка на то что пакет есть в старых ветках(oldBranches) и отсутствует в актуальной ветке. По умолчанию sisyphus
        // true если пакет есть в старых ветках и отсутствует в актуальной, иначе - false
        bool obsolescenceChecking(std::string packageName);
};