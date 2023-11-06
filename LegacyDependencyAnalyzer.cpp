#include "LegacyDependencyAnalyzer.h"


void LegacyDependencyAnalyzer::analysingBranchPackages(std::string branch) {
    packagesToAnalyse = RpmHandler::getAllPackagesName(branch);
}

std::vector<PackageDependencies> LegacyDependencyAnalyzer::getAllDependencies()
{   
    return RpmHandler::getDependenciesForPackages(packagesToAnalyse);
}

std::map<std::string,std::vector<Dependency>> LegacyDependencyAnalyzer::criteriaChecking(std::string branch)
{
    std::set<std::string> oldPackNames = getOldPackagesNames();
    auto packDependencies = RpmHandler::getDependenciesForPackages(packagesToAnalyse);

    std::map<std::string,std::vector<Dependency>> oldDepInPacks; // мапа стаарых зависимостей в пакете

    for (auto pack: packDependencies) {
        oldDepInPacks[pack.packageName] = {};

        std::vector<std::string> dependencyPacksNames = {};
        for (auto depName: pack.dependencies) {
            dependencyPacksNames.push_back(depName.dependencyName);
        }
        
        std::map<std::string, bool> checkOldDeps; //  проверка на то что пакет есть в oldPackNames
        for (auto oldDep: dependencyPacksNames) {
            if (obsolescenceChecking(oldDep)) {
                // те пакет есть в старых репозиториях и отсутствует в актуальном
                checkOldDeps[oldDep] = true;
            } else {
                checkOldDeps[oldDep] = false;
            }
        }
        
        std::string lb = branch;
        std::transform(lb.begin(), lb.end(), lb.begin(), ::tolower);
        auto depSrc = isAnythingDependsSrc(dependencyPacksNames, lb);

        for (auto oldPack: pack.dependencies) {
            bool checkOld = checkOldDeps[oldPack.dependencyName]; // true если старый, иначе false
            bool checkDepSrc = depSrc[oldPack.dependencyName]; // true если есть зависимост, иначе false

            if (checkOld && !checkDepSrc) {
                oldDepInPacks[pack.packageName].push_back(oldPack);
            }
        }
    }
    return oldDepInPacks;
}

std::set<std::string> LegacyDependencyAnalyzer::getOldPackagesNames()
{   
    std::set<std::string> oldPackages;
    for (auto br: oldBranches)
    {
        for(auto arch: classicArches) {
            auto getPack = RpmHandler::getPackageFromClassicFileName(folderClassicFiles, br, constNameClassic, arch);
            oldPackages.insert(getPack.begin(), getPack.end());
        }
    }

    for (auto name: packagesToAnalyse) {
        oldPackages.erase(name);
    }

    oldPackagesNames = oldPackages;
    return oldPackages;
}

bool LegacyDependencyAnalyzer::isAnythingDependsSrc(std::string packageName, std::string branch)
{
    auto resps = Api::divide_et_impera({packageName}, branch); // тк возвращается вектор то возьмем 0 элемент
    auto checkedPack = resps[0];

    return !checkedPack.can_delete; // те возвращаем true если есть зависимость, иначе false 
}

std::map<std::string, bool> LegacyDependencyAnalyzer::isAnythingDependsSrc(std::vector<std::string> packagesNames, std::string branch)
{   
    auto resps = Api::divide_et_impera(packagesNames, branch);
    std::map<std::string, bool>  out;
    for (auto checkedPack: resps)
    {
        out[checkedPack.name] = !checkedPack.can_delete; // те возвращаем true если есть зависимость, иначе false 
    }
    
    return out;
}

bool LegacyDependencyAnalyzer::obsolescenceChecking(std::string packageName)
{
    if (oldPackagesNames.find(packageName) != oldPackagesNames.end()) {
        return true;
    }
    return false;
}
