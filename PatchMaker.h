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

	// список пакетов для генерации патчей
	std::vector<std::string> packagesToPatch;

	// список устаревших зависимостей по названию пакета
	std::map<std::string, std::vector<std::string>> dependenciesToDelete;

	// Обрабатываемая ветка
	std::string branch;

	// Загрузка спеков из api, или из git
	void loadSpecs(specLoader sl = apiLoader);

	// Генерирует патчи для пакетов и сохраняет его по указанному пути
	void makePatch(std::string patch_destination);

private:
	std::map<std::string, std::string> specs;

	// Форматирует название типов зависимостей
	std::string PatchMaker::prepareDependencyType(std::string s);

	// Удаляет все зависимости ds из spec
	std::string generatePatch(std::string spec, std::vector<Dependency>& ds);
};