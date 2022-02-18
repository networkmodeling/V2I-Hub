cmake_minimum_required(VERSION 2.8)
set(project_name CardsCommunication)
project(${project_name} LANGUAGES CXX )

include_directories("/usr/local/include")
file(GLOB_RECURSE Demo_SOURCES *.cpp)
file(GLOB_RECURSE Demo_HEADERS *.h)

#include_directories("/usr/local/include") # SimpleAmqpClient and rabbitmq include directory
#link_libraries("/usr/local/lib/librabbitmq.so") # rabbitmq and SimpleAmqpClient lib directory
#link_libraries("/usr/local/lib/libSimpleAmqpClient.so") # rabbitmq and SimpleAmqpClient lib directory
link_libraries("/usr/local/lib/librabbitmq.a") # rabbitmq and SimpleAmqpClient lib directory
link_libraries("/usr/local/lib/libSimpleAmqpClient.a") # rabbitmq and SimpleAmqpClient lib directory

#add_library(${project_name}Static STATIC ${Demo_SOURCES} ${Demo_HEADERS}) 
#add_library(${project_name}Shared SHARED ${Demo_SOURCES} ${Demo_HEADERS}) 
#set_target_properties(${project_name}Static PROPERTIES FOLDER libs)
#set_target_properties(${project_name}Shared PROPERTIES FOLDER libs)

#add_library(${project_name} SHARED ${Demo_SOURCES} ${Demo_HEADERS}) 
add_library(${project_name} STATIC ${Demo_SOURCES} ${Demo_HEADERS})
set_target_properties(${project_name} PROPERTIES OUTPUT_NAME ${project_name})

#target_link_libraries(${PROJECT_NAME} rabbitmq SimpleAmqpClient) # add rabbitmq lib and SimpleAmqpClient lib