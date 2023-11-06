#include "LegacyDependencyAnalyzer.h"


void LegacyDependencyAnalyzer::analysingBranchPackages(std::string branch) {
    packagesToAnalyse = RpmHandler::getAllPackagesName(branch);
}

std::vector<PackageDependencies> LegacyDependencyAnalyzer::getAllDependencies()
{   
    return RpmHandler::getDependenciesForPackages(packagesToAnalyse);
}

void LegacyDependencyAnalyzer::criteriaChecking()
{
    getOldPackagesNames();
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
