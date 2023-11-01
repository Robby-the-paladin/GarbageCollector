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

bool LegacyDependencyAnalyzer::obsolescenceChecking(std::string packageName)
{
    if (oldPackagesNames.find(packageName) != oldPackagesNames.end()) {
        return true;
    }
    return false;
}
