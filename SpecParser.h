#pragma once
#include <stdlib.h>
#include <regex>
#include <iostream>
#include <set>
#include <fstream>

class SpecParser {
public:
	SpecParser() {}

	std::set<std::string> getDeprecatedPackages(std::string specfile);
	std::set<std::string> getBuildRequiresPrePackages(std::string specfile);
};