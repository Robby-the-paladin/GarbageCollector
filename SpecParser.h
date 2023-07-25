#pragma once
#include <stdlib.h>
#include <regex>
#include <iostream>
#include <set>
#include <fstream>

class SpecParser {
public:
	struct lib_data {
		std::string name;
		std::string sign;
		std::string version;
		std::string type;
	};
	SpecParser() {}

    int error = 0;

	std::set<std::string> getDeprecatedPackages(std::string specfile);
	std::set<std::string> getBuildRequiresPrePackages(std::string specfile);

	std::set<std::string> getDeprecatedPackages_test(std::string specfile);

	lib_data* strToStruct_lib_data(std::string data);
	std::vector<lib_data> strToStructSet_lib_data(std::set<std::string> libs);
};

