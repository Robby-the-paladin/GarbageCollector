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

	// ������ ������� ��� ��������� ������
	std::vector<std::string> packagesToPatch;

	// ������ ���������� ������������ �� �������� ������
	std::map<std::string, std::vector<std::string>> dependenciesToDelete;

	// �������������� �����
	std::string branch;

	// �������� ������ �� api, ��� �� git
	void loadSpecs(specLoader sl = apiLoader);

	// ���������� ���� �� �������� ������ � ��������� ��� �� ���������� ����
	void makePatch(std::string package, std::string patch_destination);

private:
	std::map<std::string, std::string> specs;
};