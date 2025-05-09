cmake_minimum_required(VERSION 3.10)
project(step1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_library(LIMESUITE_LIBRARY NAMES LimeSuite)
find_path(LIMESUITE_INCLUDE_DIR NAMES LimeSuite.h PATH_SUFFIXES lime)

if (NOT LIMESUITE_LIBRARY OR NOT LIMESUITE_INCLUDE_DIR)
    message(FATAL_ERROR "LimeSuite not found")
endif()

add_executable(step1 step1.cpp)
target_include_directories(step1 PRIVATE ${LIMESUITE_INCLUDE_DIR})
target_link_libraries(step1 PRIVATE ${LIMESUITE_LIBRARY})