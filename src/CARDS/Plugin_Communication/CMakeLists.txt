cmake_minimum_required(VERSION 2.8)
set(project_name PluginCommunication)
project(${project_name} LANGUAGES CXX )

add_definitions("-DLINUX")

include_directories("/usr/local/include")
include_directories("/home/yingtong/V2I-Hub/src/CARDS/CARDS_Communication")
include_directories("/home/yingtong/V2I-Hub/src/CARDS/Utility")
include_directories("/home/yingtong/V2I-Hub/src/CARDS/jsoncpp/include") 
include_directories("/home/yingtong/V2I-Hub/src/CARDS/IO_Libs")


file(GLOB_RECURSE Demo_SOURCES *.cpp)
file(GLOB_RECURSE Demo_HEADERS *.h)

link_libraries("/usr/local/lib/librabbitmq.so") # rabbitmq and SimpleAmqpClient lib directory
link_libraries("/usr/local/lib/libSimpleAmqpClient.so") # rabbitmq and SimpleAmqpClient lib directory
link_libraries("/home/yingtong/V2I-Hub/src/CARDS/CARDS_Communication/build/libCardsCommunication.so") 
link_libraries("/home/yingtong/V2I-Hub/src/CARDS/Utility/build/libUtility.so") 
link_libraries("/home/yingtong/V2I-Hub/src/CARDS/jsoncpp/build/debug/lib/libjsoncpp.so")
link_libraries("/home/yingtong/V2I-Hub/src/CARDS/IO_Libs/build/libIOLibs.so") 

add_library(${project_name} SHARED ${Demo_SOURCES} ${Demo_HEADERS}) 
set_target_properties(${project_name} PROPERTIES OUTPUT_NAME ${project_name})