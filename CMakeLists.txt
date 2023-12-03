cmake_minimum_required(VERSION 3.20)
project(GarbageCollector)

set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

find_package(CURL REQUIRED)
find_package(jsoncpp)
find_package(PkgConfig REQUIRED)

# find_package(libpqxx REQUIRED)
find_package(Boost)
IF (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
endif()

# find_library(PQXX_LIB pqxx)
# find_library(PQ_LIB pq)
#find_package(libpqxx REQUIRED)
find_path(libpqxx_INCLUDE_DIR /pqxx/pqxx)
include(SelectLibraryConfigurations)
find_library(libpqxx_LIBRARY_RELEASE NAMES libpqxx)
find_library(libpqxx_LIBRARY_DEBUG NAMES libpqxxd)
select_library_configurations(libpqxx)
#find_package(PostgreSQL)

pkg_check_modules(JSONCPP jsoncpp)
link_libraries(${JSONCPP_LIBRARIES})

add_executable(GarbageCollector
        main.cpp
        "base64.h" "base64.cpp"
        "Cacher.h" "Cacher.cpp"
        "Api.h" "Api.cpp"
        "PostgreHandler.h" "PostgreHandler.cpp" 
        "LegacyDependencyAnalyzer.h" "LegacyDependencyAnalyzer.cpp"
        "RpmHandler.h" "RpmHandler.cpp"
        "PatchMaker.h" "PatchMaker.cpp"
        "Config.h" "Config.cpp"
        "Aux.h"
        "WDS.h" "WDS.cpp"
         )

include_directories(${JSONCPP_INCLUDE_DIR})
message("JSONINC:" ${JSONCPP_INCLUDE_DIR})
target_link_libraries(GarbageCollector jsoncpp_lib)
message("JSONLIB:" ${JSONCPP_LIBRARIES})
include_directories(${CURL_INCLUDE_DIR})
target_link_libraries(GarbageCollector ${CURL_LIBRARIES} )
target_link_libraries(GarbageCollector -lpqxx -lrpm -lrpmio -lhtmlcxx)