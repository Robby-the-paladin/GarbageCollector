cmake_minimum_required(VERSION 3.20)
project(GarbageCollector)

set(CMAKE_CXX_STANDARD 17)

find_package(CURL REQUIRED)
find_package(jsoncpp)
find_package(PkgConfig REQUIRED)
find_package(boost REQUIRED)
find_package(libmongocxx REQUIRED)
find_package(libbsoncxx REQUIRED)
include_directories( ${BOOST_INCLUDE_DIR} )
include_directories(${LIBMONGOCXX_INCLUDE_DIRS} ${LIBBSONCXX_INCLUDE_DIRS})
message("LIBMONGOCXXINC:" ${LIBMONGOCXX_INCLUDE_DIRS})
pkg_check_modules(JSONCPP jsoncpp)
link_libraries(${JSONCPP_LIBRARIES})

add_executable(GarbageCollector
        main.cpp "SpecCollector.h" "SpecCollector.cpp" "base64.h" "base64.cpp" "Api.h" "Api.cpp" "SpecParser.h" "SpecParser.cpp")

include_directories(${JSONCPP_INCLUDE_DIR})
message("JSONINC:" ${JSONCPP_INCLUDE_DIR})
target_link_libraries(GarbageCollector jsoncpp_lib)
message("JSONLIB:" ${JSONCPP_LIBRARIES})
include_directories(${CURL_INCLUDE_DIR})
target_link_libraries(GarbageCollector ${BOOST_LIBRARIES} )
target_link_libraries(GarbageCollector ${CURL_LIBRARIES} )
target_link_libraries(GarbageCollector ${LIBMONGOCXX_LIBRARIES})
target_link_libraries(GarbageCollector ${LIBBSONCXX_LIBRARIES})