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
#include "Api.h"
#include "WDS.h"



class LegacyDependencyAnalyzer {
    public:        
        // список старых веток для анализа
        std::vector<std::string> oldBranches = {"4.0", "4.1", "p5", "p6", "p7", "p8"};

        // статичная часть названия классик файла pkglist.classic(архитектура x86_64)  
        const std::string folderClassicFiles = "oldBranches";
        std::string constNameClassic = "pkglist.classic."; 
        std::vector<std::string> classicArches = {"x86_64", "noarch"};

        // список пакетов для анализа
        std::map<std::string, std::pair<std::string, std::string>> packagesToAnalyse;

        // список пакетов для фикса
        std::vector<std::string> packagesToFix;

        LegacyDependencyAnalyzer(){};

        // устанавливает список пакетов для анализа из ветки. По умолчанию sisyphus
        void analysingBranchPackages(std::set<std::string> packNames, std::string branch = "Sisyphus");

        // возвращает список всех зависимостей для packagesToAnalyse
        std::vector<PackageDependencies> getAllDependencies();

        // проверяет зависимости из unicDependecies на критерии (присутствие в старых репозиториях и неиспользуемость в актульном)
        std::map<std::string,std::vector<Dependency>> criteriaChecking(Cacher& ch, std::string branch = "Sisyphus");
    
        // возвращает все provides, которые есть в oldBranches и отсутствуют в проверяемой ветке
        std::set<std::string> getOldProvides();
    private:
        // список уникальных зависимостей генерируемых методом getAllDependencies
        std::set<Dependency> unicDependecies;

        //
        std::set<std::string> oldPackagesNames;
        // список уникальных старых provides, которых нет в проверяемой ветке
        std::set<std::string> oldDepProvides;

        // возвращает множество пакетов, которые есть в oldBranches и отсутствуют в проверяемой ветке
        std::set<std::string> getOldPackagesNames();

        // проверка на то что в актуальной ветке нет пакетов, которые зависят от исследуемого
        // true если есть зависимость, false - иначе
        bool isAnythingDependsSrc(std::string packageName, std::string branch, Cacher& ch);

        // проверка на то что в актуальной ветке нет пакетов, которые зависят от исследуемых и возвращает 
        // ответ для каждого пакета. 
        // true если есть зависимость, false - иначе
        std::map<std::string, bool> isAnythingDependsSrc(std::vector<std::string> packagesNames, std::string branch, Cacher& ch);

        // проверка на то что пакет есть в старых ветках(oldBranches) и отсутствует в актуальной ветке. По умолчанию sisyphus
        // true если пакет есть в старых ветках и отсутствует в актуальной, иначе - false
        bool obsolescenceChecking(std::string packageName);
};