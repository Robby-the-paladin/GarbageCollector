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
        main.cpp "SpecCollector.h" "SpecCollector.cpp" "base64.h" "base64.cpp" "Api.h" "Api.cpp" "SpecParser.h" "SpecParser.cpp" "PostgreHandler.h" "PostgreHandler.cpp")

include_directories(${JSONCPP_INCLUDE_DIR})
message("JSONINC:" ${JSONCPP_INCLUDE_DIR})
target_link_libraries(GarbageCollector jsoncpp_lib)
message("JSONLIB:" ${JSONCPP_LIBRARIES})
include_directories(${CURL_INCLUDE_DIR})
#include_directories(${PostgreSQL_INCLUDE_DIR})
target_link_libraries(GarbageCollector ${CURL_LIBRARIES} )
#target_link_libraries(GarbageCollector ${PostrgreSQL_LIBRARY} )
target_link_libraries(GarbageCollector -lpqxx )
# TARGET_LINK_LIBRARIES(GarbageCollector ${PQXX_LIB} ${PQ_LIB})
#target_link_libraries(GarbageCollector libpqxx::pqxx_shared)
#target_link_libraries(GarbageCollector ${libpqxx_LIBRARIES} ${PQ_LIBRARIES})