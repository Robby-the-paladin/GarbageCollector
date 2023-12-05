#pragma once

#include <iostream>
#include <unistd.h>
#include <set>
#include <map>
#include <vector>
#include <errno.h>
#include <fcntl.h>
#include <rpm/rpmlib.h>
#include <rpm/header.h>
#include <rpm/rpmdb.h>
#include <curl/curl.h>
#include "Aux.h"

// структура зависимость
// dependencyName - название зависимости
// type - тип зависимости
// version - версия пакета в зависимости (пока не используется)
// sign - знак сравнения версии (пока не используется)
struct Dependency
{   
    // название пакета зависимости
    std::string dependencyName;
    // тип зависимости
    std::string type;
    // версия пакета в зависимости (пока не используется)
    std::string version;
    // знак сравнения версии (пока не используется)
    std::string sign;
};

//  packageName - имя пакета
//  dependencies - набор зависимостей в его спек файде
struct PackageDependencies
{
    // имя пакета
    std::string packageName;
    // релиз
    std::string release;
    // набор зависимостей в его спек файле
    std::vector<Dependency> dependencies;
};

class RpmHandler
{
public:
    // список classic файлов, описывающих актуальную ветку
    std::string static constNameClassic;
    std::vector<std::string> static classicArches;
    
    RpmHandler(){};

    // возвращает список всех пакетов в branch
    std::map<std::string, std::pair<std::string, std::string>> static getAllPackagesName(std::string branch, std::set<std::string> packNames);

    // возвращает набор зависимостей для списка пакетов
    std::vector<PackageDependencies> static getDependenciesForPackages(std::map<std::string, std::pair<std::string, std::string>> packageList);

    // возвращает множество пакетов по названию классик файла
    std::set<std::string> static getPackageFromClassicFileName(std::string folder, std::string branch, 
                                                               std::string classicName, std::string arch);

    // возвращает все провайдсы из старых веток (classicArches)
    std::set<std::string> static getAllProvides(std::string folder, std::string branch, 
                                                   std::string classicName, std::string arch);

    std::map<std::string, std::set<std::string>> static packagesProvides();
private:
    // возвращает файловый дескриптор
    FD_t static getCalssicFileDescriptor(std::string fileName);

    // из множества строк зависимостей возвращает список структур Dependecy 
    std::vector<Dependency> static setStrToStructsDependencies(std::set<std::string> stringDependencies);

    // из строки парсит структуру Dependency
    Dependency static strToStructDependency(std::string data);

    // загружает pkglist для конкретного branch и ирхитектуры
    void static downloadClassicFile(std::string branch, std::string arch, std::string path = "");
};
