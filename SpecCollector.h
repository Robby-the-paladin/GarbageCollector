#pragma once
#include <iostream>
#include <curl/curl.h>
#include <json/json.h>
#include "base64.h"

class SpecCollector {
public:
	SpecCollector() {};

	std::string getSpec(std::string branch, std::string name);
};