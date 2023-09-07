#pragma once
#include <stdlib.h>
#include <regex>
#include <iostream>
#include <set>
#include <fstream>
#include <map>

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

	std::set<std::string> getBuildRequiresPrePackages(std::string specfile);

	std::set<std::string> getDeprecatedPackages_test(std::string specfile);

	static lib_data strToStruct_lib_data(std::string data);
	static std::vector<lib_data> strToStructSet_lib_data(std::set<std::string> libs);
	static std::string structTostr_lib_data(lib_data data);
};

