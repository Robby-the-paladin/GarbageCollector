#include <iostream>
#include "SpecCollector.h"
#include "Api.h"

int main() {
    std::cout << "Edgar genius!" << std::endl;
    SpecCollector s;
    s.getSpec("p10", "boost");

    Api api;
    api.checkPackage("");    
    return 0;
}