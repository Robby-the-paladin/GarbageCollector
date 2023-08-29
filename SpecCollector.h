#pragma once
#include <iostream>
#include <curl/curl.h>
#include <json/json.h>
#include <vector>
#include "Api.h"
#include "base64.h"

class SpecCollector {
public:
	SpecCollector() {};

	std::string getSpec(std::string branch, std::string name);
	std::string getSpecDate(std::string branch, std::string name);
	std::vector<std::string> getBranchPackageNames(std::string branch);

private:
	Api::response getSpecResponse(std::string branch, std::string name);
};