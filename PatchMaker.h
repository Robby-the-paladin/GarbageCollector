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
#include "SpecCollector.h"

class PatchMaker {
public:
	enum class specLoader {
		apiLoader,
		gitLoader
	};

	PatchMaker() {};

	// список пакетов дл€ генерации патчей
	std::vector<std::string> packagesToPatch;

	// список устаревших зависимостей по названию пакета
	std::map<std::string, std::vector<std::string>> dependenciesToDelete;

	// ќбрабатываема€ ветка
	std::string branch;

	// «агрузка спеков из api, или из git
	void loadSpecs(specLoader sl = apiLoader);

	// √енерирует патч по названию пакета и сохран€ет его по указанному пути
	void makePatch(std::string package, std::string patch_destination);

private:
	std::map<std::string, std::string> specs;
};