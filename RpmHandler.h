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

// структура зависимость
// dependencyName - название зависимости
// type - тип зависимости
// version - версия пакета в зависимости (пока не используется)
// sign - знак сравнения версии (пока не используется)
struct Dependency
{       
    std::string dependencyName;
    std::string type;
    std::string version;
    std::string sign;
};

//  packageName - имя пакета
//  dependencies - набор зависимостей в его спек файде
struct PackageDependencies
{
    std::string packageName;
    std::vector<Dependency> dependencies;
};

class RpmHandler
{
public:
    // список classic файлов, описывающих актуальную ветку
    std::vector<std::string> classicFilesNames = {"pkglist.classic.x86_64", "pkglist.classic.noarch"};
    
    RpmHandler(){};

    // возвращает список всех пакетов в branch
    std::vector<std::string> static getAllPackagesName(std::string branch);

    // возвращает набор зависимостей для списка пакетов
    std::vector<PackageDependencies> static getDependenciesForPackages(std::vector<std::string> packageList);

    // возвращает множество пакетов по названию классик файла
    std::set<std::string> static getPackageFromClassicFileName(std::string fileName);
private:
    // возвращает файловый дескриптор
    FD_t getCalssicFileDescriptor(std::string fileName);

    // из множества строк зависимостей возвращает список структур Dependecy 
    std::vector<Dependency> setStrToStructsDependencies(std::set<std::string> stringDependencies);

    // из строки парсит структуру Dependency
    Dependency strToStructDependency(std::string data);
};
