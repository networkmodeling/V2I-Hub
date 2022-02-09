PROJECT ( HRIStatusPlugin VERSION 5.0 LANGUAGES CXX )

SET (TMX_PLUGIN_NAME "HRIStatus")

FIND_PACKAGE (XercesC REQUIRED)
FIND_PACKAGE (NetSNMP REQUIRED)

BuildTmxPlugin ()

TARGET_INCLUDE_DIRECTORIES ( ${PROJECT_NAME} PUBLIC ${XercesC_INCLUDE_DIRS} ${NETSNMP_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES ( ${PROJECT_NAME} tmxutils ${XercesC_LIBRARY} ${NETSNMP_LIBRARIES})

add_definitions("-DLINUX")

# tyt add

include_directories("/usr/local/include") # SimpleAmqpClient and rabbitmq include directory
link_libraries("/usr/local/lib/x86_64-linux-gnu") # rabbitmq and SimpleAmqpClient lib directory
link_libraries("/usr/local/lib") # rabbitmq and SimpleAmqpClient lib directory

