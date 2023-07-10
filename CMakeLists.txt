cmake_minimum_required(VERSION 3.20)
project(GarbageCollector)

set(CMAKE_CXX_STANDARD 17)

find_package(CURL REQUIRED)
find_package(jsoncpp)
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
include_directories(${JSONCPP_INCLUDE_DIR})
message("JSONINC:" ${JSONCPP_INCLUDE_DIR})
link_libraries(${JSONCPP_LIBRARIES})

add_executable(GarbageCollector
        main.cpp "SpecCollector.h" "SpecCollector.cpp" "base64.h" "base64.cpp")


target_link_libraries(GarbageCollector ${JSONCPP_LIBRARIES})
message("JSONLIB:" ${JSONCPP_LIBRARIES})
include_directories(${CURL_INCLUDE_DIR})
target_link_libraries(GarbageCollector ${CURL_LIBRARIES} )