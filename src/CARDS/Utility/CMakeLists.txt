cmake_minimum_required(VERSION 2.8)
set(project_name Utility)
project(${project_name} LANGUAGES CXX )

add_definitions("-DLINUX")

include_directories("/usr/local/include")
file(GLOB_RECURSE Demo_SOURCES *.cpp)
file(GLOB_RECURSE Demo_HEADERS *.h)

# add_library(${project_name} SHARED ${Demo_SOURCES} ${Demo_HEADERS}) 
add_library(${project_name} STATIC ${Demo_SOURCES} ${Demo_HEADERS}) 
set_target_properties(${project_name} PROPERTIES OUTPUT_NAME ${project_name})