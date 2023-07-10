#pragma once
#include <iostream>
#include <curl.h>
#include "base64.h"

class SpecCollector {
public:
	SpecCollector() {};

	std::string getSpec(std::string branch, std::string name);
};