#pragma once

#include <iostream>
#include <regex>
#include <unistd.h>
#include <set>
#include <map>
#include <queue>
#include <thread>
#include <mutex>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <filesystem>
#include <fstream>
#include "RpmHandler.h"
#include "Api.h"

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
	std::map<std::string, std::vector<Dependency>> dependenciesToDelete;

	// Обрабатываемая ветка
	std::string branch = "sisyphus";

	// Загрузка спеков из api, или из git
	void loadSpecs(specLoader sl = specLoader::apiLoader);

	// Генерирует патчи для пакетов и сохраняет его по указанному пути
	void makePatch(std::string patch_destination);

private:
	std::map<std::string, std::string> specs;

	// Форматирует название типов зависимостей
	std::string prepareDependencyType(std::string s);

	// Удаляет все зависимости ds из spec
	std::string generatePatch(std::string spec, std::vector<Dependency>& ds);

	// инкриментирует реелиз
	std::string incriment_release(std::string release);
};