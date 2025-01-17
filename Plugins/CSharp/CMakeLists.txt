cmake_minimum_required(VERSION 3.12)

set(PLUGIN_NAME "Klemmgine.CSharp.Loader")

add_subdirectory("../PluginInterface" "PluginInterface")
set(SRC_DIR "Loader")

include("FindSdk.cmake")

file(
	GLOB_RECURSE
	SRCS
	"${SRC_DIR}/**.cpp"
)
file(
	GLOB_RECURSE
	INCLUDES
	"${SRC_DIR}/**.h"
)

add_library(${PLUGIN_NAME} SHARED ${INCLUDES} ${SRCS})
target_link_libraries(${PLUGIN_NAME} PRIVATE KlemmginePlugin)
target_include_directories(${PLUGIN_NAME} PRIVATE ${NET_SDK_BINARY_DIR})
if(WIN32)
	file(COPY "${NET_SDK_BINARY_DIR}/nethost.dll" DESTINATION "${CMAKE_BINARY_DIR}/bin")
	target_link_libraries(${PLUGIN_NAME} PRIVATE "${NET_SDK_BINARY_DIR}/nethost.lib")
else()
	file(COPY "${NET_SDK_BINARY_DIR}/libnethost.so" DESTINATION "${CMAKE_BINARY_DIR}/bin")
	target_link_libraries(${PLUGIN_NAME} PRIVATE "${CMAKE_BINARY_DIR}/bin/libnethost.so")
	target_link_options(${PLUGIN_NAME} PRIVATE "-Wl,-rpath,$ORIGIN")
endif()

add_dependencies(Klemmgine ${PLUGIN_NAME})