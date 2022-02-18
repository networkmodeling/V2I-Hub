PROJECT ( HRIPredictedSpatPlugin VERSION 5.0 LANGUAGES CXX )

SET (TMX_PLUGIN_NAME "HRIPredictedSpatPlugin")

FIND_PACKAGE (XercesC REQUIRED)
FIND_PACKAGE (NetSNMP REQUIRED)

BuildTmxPlugin ()

TARGET_INCLUDE_DIRECTORIES ( ${PROJECT_NAME} PUBLIC ${XercesC_INCLUDE_DIRS} ${NETSNMP_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES ( ${PROJECT_NAME} tmxutils ${XercesC_LIBRARY} ${NETSNMP_LIBRARIES})

add_definitions("-DLINUX")

# tyt add
include_directories("/home/yingtong/V2I-Hub/src/CARDS/Plugin_Communication") 
include_directories("/home/yingtong/V2I-Hub/src/CARDS/CARDS_Communication") 
include_directories("/home/yingtong/V2I-Hub/src/CARDS/Utility") 
include_directories("/home/yingtong/V2I-Hub/src/CARDS/IO_Libs") 
include_directories("/home/yingtong/V2I-Hub/src/CARDS/jsoncpp/include") 
include_directories("/usr/local/include") # SimpleAmqpClient and rabbitmq include directory
link_libraries("/usr/local/lib") # rabbitmq and SimpleAmqpClient lib directory
#target_link_libraries(${PROJECT_NAME} /home/yingtong/V2I-Hub/src/CARDS/Plugin_Communication/build/libPluginCommunication.so)
#target_link_libraries(${PROJECT_NAME} /home/yingtong/V2I-Hub/src/CARDS/CARDS_Communication/build/libCardsCommunication.so)
#target_link_libraries(${PROJECT_NAME} /home/yingtong/V2I-Hub/src/CARDS/Utility/build/libUtility.so)
#target_link_libraries(${PROJECT_NAME} /home/yingtong/V2I-Hub/src/CARDS/IO_Libs/build/libIOLibs.so)
#target_link_libraries(${PROJECT_NAME} /home/yingtong/V2I-Hub/src/CARDS/jsoncpp/build/debug/lib/libjsoncpp.so)
#target_link_libraries(${PROJECT_NAME} rabbitmq SimpleAmqpClient) # add rabbitmq lib and SimpleAmqpClient lib

target_link_libraries(${PROJECT_NAME} /home/yingtong/V2I-Hub/src/CARDS/Plugin_Communication/build/libPluginCommunication.a)
target_link_libraries(${PROJECT_NAME} /home/yingtong/V2I-Hub/src/CARDS/CARDS_Communication/build/libCardsCommunication.a)
target_link_libraries(${PROJECT_NAME} /home/yingtong/V2I-Hub/src/CARDS/Utility/build/libUtility.a)
target_link_libraries(${PROJECT_NAME} /home/yingtong/V2I-Hub/src/CARDS/IO_Libs/build/libIOLibs.a)
target_link_libraries(${PROJECT_NAME} /home/yingtong/V2I-Hub/src/CARDS/jsoncpp/build/debug/lib/libjsoncpp.a)
target_link_libraries(${PROJECT_NAME} rabbitmq SimpleAmqpClient) # add rabbitmq lib and SimpleAmqpClient lib

