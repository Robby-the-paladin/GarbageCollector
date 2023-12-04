#include "LegacyDependencyAnalyzer.h"


void LegacyDependencyAnalyzer::analysingBranchPackages(std::set<std::string> packNames, std::string branch) {
    packagesToAnalyse = RpmHandler::getAllPackagesName(branch, packNames);
}

std::vector<PackageDependencies> LegacyDependencyAnalyzer::getAllDependencies()
{   
    return RpmHandler::getDependenciesForPackages(packagesToAnalyse);
}

std::map<std::string,std::vector<Dependency>> LegacyDependencyAnalyzer::criteriaChecking(Cacher& ch, std::string branch)
{
    std::set<std::string> oldPackNames = getOldPackagesNames();
    getOldProvides();
    auto packDependencies = RpmHandler::getDependenciesForPackages(packagesToAnalyse);

    std::map<std::string,std::vector<Dependency>> oldDepInPacks; // мапа стаарых зависимостей в пакете

    std::set<std::string> fix;
    for (auto pack: packDependencies) {
        oldDepInPacks[pack.packageName] = {};

        std::vector<std::string> dependencyPacksNames = {};
        for (auto depName: pack.dependencies) {
            dependencyPacksNames.push_back(depName.dependencyName);
        }
        
        std::map<std::string, bool> checkOldDeps; //  проверка на то что пакет есть в oldPackNames
        for (auto oldDep: dependencyPacksNames) { // oldDepProvides
            if (Aux::is_virtual(oldDep)) {
                // если виртуальный пакет был кем-то провайден в старых репозиториях и отсутствует в актуальном
                checkOldDeps[oldDep] = oldDepProvides.count(oldDep);
            } else {
                // те пакет есть в старых репозиториях и отсутствует в актуальном
                checkOldDeps[oldDep] = obsolescenceChecking(oldDep);
            }
        }
        
        std::string lb = branch;
        std::transform(lb.begin(), lb.end(), lb.begin(), ::tolower);
        auto depSrc = isAnythingDependsSrc(dependencyPacksNames, lb, ch);

        std::cout <<  pack.dependencies.size() << std::endl;
        for (auto oldPack: pack.dependencies) {
            bool checkOld = checkOldDeps[oldPack.dependencyName]; // true если старый, иначе false
            bool checkDepSrc = depSrc[oldPack.dependencyName]; // true если есть зависимость, иначе false

            std::cout << oldPack.dependencyName << " " << oldPack.type << " " << "Is old? " << checkOld << " Is depend..? " << checkDepSrc << std::endl; 
            if (checkOld && !checkDepSrc) {
                oldDepInPacks[pack.packageName].push_back(oldPack);
                std::cout << pack.packageName << " delete => " << oldPack.dependencyName << " " << oldPack.type << std::endl;
                fix.insert(pack.packageName);
            }
        }
    }
    // pakagesToFix = std::vector<std::string>(fix.begin(), fix.end());
    packagesToFix.resize(fix.size());
    std::copy(fix.begin(), fix.end(), packagesToFix.begin());
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
        oldPackages.erase(name.first);
    }

    oldPackagesNames = oldPackages;
    return oldPackages;
}

std::set<std::string> LegacyDependencyAnalyzer::getOldProvides()
{   
    std::set<std::string> oldProvides;
    for (auto br: oldBranches)
    {
        for(auto arch: classicArches) {
            auto getPack = RpmHandler::getAllProvides(folderClassicFiles, br, constNameClassic, arch);
            oldProvides.insert(getPack.begin(), getPack.end());
        }
    }

    auto dep =  RpmHandler::getDependenciesForPackages(packagesToAnalyse);
    for (auto pack: dep) {
        for (auto dep: pack.dependencies) {
            if (dep.type == "provides")
                oldProvides.erase(dep.dependencyName);
        }
    }

    oldDepProvides = oldProvides;
    return oldProvides;
}

bool LegacyDependencyAnalyzer::isAnythingDependsSrc(std::string packageName, std::string branch, Cacher& ch)
{
    auto resps = Api::divide_et_impera({packageName}, branch, ch); // тк возвращается вектор то возьмем 0 элемент
    auto checkedPack = resps[0];

    return !checkedPack.can_delete; // те возвращаем true если есть зависимость, иначе false 
}

std::map<std::string, bool> LegacyDependencyAnalyzer::isAnythingDependsSrc(std::vector<std::string> packagesNames, std::string branch, Cacher& ch)
{   
    // auto resps = Api::divide_et_impera(packagesNames, branch, ch);
    auto resps = wds::has_active_dependencies(packagesNames, ch);

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
