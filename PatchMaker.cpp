#include "PatchMaker.h"

// Загрузка спеков из api, или из git
void PatchMaker::loadSpecs(specLoader sl) {
	if (sl == specLoader::apiLoader) {
		for (int i = 0; i < packagesToPatch.size(); i++) {
            std::cout << "Loading spec for " << packagesToPatch[i] << std::endl;
            auto spec = Api::getSpecFile(branch, packagesToPatch[i]);
            if (spec.has_value())
			    specs[packagesToPatch[i]] = spec.value();
		}
	}
	if (sl == specLoader::gitLoader) {
		// TODO: сделать загрузку с гита
	}
}

// Форматирует название типов зависимостей
std::string PatchMaker::prepareDependencyType(std::string s) {
    std::transform(s.begin(), s.begin() + 1, s.begin(), ::toupper);
    std::transform(s.begin() + 1, s.end(), s.begin() + 1, ::tolower);
    return s + ":";
}

// Удаляет все зависимости ds из spec
std::string PatchMaker::generatePatch(std::string spec, std::vector<Dependency>& ds) {
    // Временное исключение changelog, для корректной работы regex replace
    std::string changelog = spec.substr(spec.find("%changelog"));
    spec = spec.substr(0, spec.find("%changelog"));

    for (int i = 0; i < ds.size(); i++) {
        // Запускаем пока изменяется спек
        auto old_spec = spec;
        do {
            old_spec = spec;
            std::string deptype = prepareDependencyType(ds[i].type);
            std::string version = "( " + ds[i].sign + " " + ds[i].version + ")";
            std::string ex;

            if (ds[i].version == "") {
                version = "([^\n])*";
            }

            ex = "(" + prepareDependencyType(ds[i].type) + " )";
            ex += "(" + ds[i].dependencyName + ")";
            ex += version + "(\\r|\\n|\\0)";
            std::cout << ex << std::endl;
            std::regex expr2(ex, std::regex::icase);
            spec = std::regex_replace(spec, expr2, "");

            ex = "(" + prepareDependencyType(ds[i].type) + " .*)";
            ex += "(, " + ds[i].dependencyName + ")";
            ex += version + "(\\r|\\n|\\0)";
            std::cout << ex << std::endl;
            std::regex expr3(ex, std::regex::icase);
            spec = std::regex_replace(spec, expr3, "$1\n");

            if (ds[i].version == "") {
                version = "(( )*(<|>|=|<=|>=)( )+[^ ,\n]*)";
            }        

            ex = "(" + prepareDependencyType(ds[i].type) + " .*)";
            ex += "(" + ds[i].dependencyName + ")";
            ex += version + "((, )|( ))";
            std::cout << ex << std::endl;
            std::regex expr(ex, std::regex::icase);
            spec = std::regex_replace(spec, expr, "$1");
        } while(old_spec != spec);
    }

    return spec + changelog;
}

// Генерирует патчи для пакетов и сохраняет его по указанному пути
void PatchMaker::makePatch(std::string patch_destination) {
	for (int i = 0; i < packagesToPatch.size(); i++) {
        std::cout << "Patching package " + packagesToPatch[i] << "\n";
        std::ofstream spec;
        spec.open(packagesToPatch[i] + "~.spec");
		spec << specs[packagesToPatch[i]];

        std::ofstream patched;
        patched.open(packagesToPatch[i] + ".spec");

        // ИГНОРИРУЕМ ВЕРСИИ ДО ЛУЧШИХ ВРЕМЕН!!!!
        for (int j = 0; j < dependenciesToDelete[packagesToPatch[i]].size(); j++) {
             dependenciesToDelete[packagesToPatch[i]][j].version = "";
        }
        // 

        std::string patched_str = generatePatch(specs[packagesToPatch[i]], dependenciesToDelete[packagesToPatch[i]]);
        patched << patched_str;

        if (patched_str != specs[packagesToPatch[i]]) {
            spec.close();
            patched.close();
            std::cout << "diff -Naru " + packagesToPatch[i] + ".spec~ " + packagesToPatch[i] + ".spec > " + patch_destination + packagesToPatch[i] + ".patch" << std::endl;
            int resp = system(("diff -Naru Specfile.spec PatchedSpecfile.spec > " + patch_destination + packagesToPatch[i] + ".patch").c_str());
            if (resp != 0) {
                std::cout << "Error while making patch for " << packagesToPatch[i] << ": " << resp << std::endl;
                continue;
            }
        } else {
            std::cout << "Patch not needed" << std::endl;
        }
	}
}