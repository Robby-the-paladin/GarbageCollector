#include "LegacyDependencyAnalyzer.h"


void LegacyDependencyAnalyzer::analysingBranchPackages(std::string branch = "sisyphus") {
    packagesToAnalyse = RpmHandler::getAllPackagesName(branch);
}

std::vector<PackageDependencies> LegacyDependencyAnalyzer::getAllDependencies()
{   
    return RpmHandler::getDependenciesForPackages(packagesToAnalyse);
}

std::vector<std::string> LegacyDependencyAnalyzer::getOldPackagesNames()
{   
    std::set<std::string> oldPackages;
    for (auto br: oldBranches)
    {
        std::string path = folderClassicFiles + br + "/";
        for(auto fileName: classicFilesNames) {
            // path + fileName;
            auto getPack = RpmHandler::getPackageFromClassicFileName(path + fileName);
            oldBranches.insert(getPack.begin(), getPack.end());
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
