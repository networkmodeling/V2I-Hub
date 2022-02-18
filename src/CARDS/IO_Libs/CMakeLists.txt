cmake_minimum_required(VERSION 2.8)
set(project_name IOLibs)
project(${project_name} LANGUAGES CXX )

add_definitions("-DLINUX")

include_directories("/usr/local/include")
include_directories("/home/yingtong/V2I-Hub/src/CARDS/jsoncpp/include") 

file(GLOB_RECURSE Demo_SOURCES *.cpp)
file(GLOB_RECURSE Demo_HEADERS *.h)

#link_libraries("/home/yingtong/V2I-Hub/src/CARDS/jsoncpp/build/debug/lib/libjsoncpp.so")
link_libraries("/home/yingtong/V2I-Hub/src/CARDS/jsoncpp/build/debug/lib/libjsoncpp.a")

#add_library(${project_name} SHARED ${Demo_SOURCES} ${Demo_HEADERS}) 
add_library(${project_name} STATIC ${Demo_SOURCES} ${Demo_HEADERS}) 
set_target_properties(${project_name} PROPERTIES OUTPUT_NAME ${project_name})