cmake_minimum_required(VERSION 3.12)

set(PLUGIN_NAME "Klemmgine.CSharp.Loader")

set(SRC_DIR "Loader")

include("FindSdk.cmake")

if(NOT NET_SDK_BINARY_DIR)
	message(".NET sdk not found, not compiling c# plugin...")
	return()
endif()

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

klemmgine_plugin_dirs()

add_library(${PLUGIN_NAME} SHARED ${INCLUDES} ${SRCS} "Loader/Resources.rc")

if(WIN32)
	foreach(LANG  C CXX RC)
		set(CMAKE_${LANG}_STANDARD_INCLUDE_DIRECTORIES ${CUSTOM_INCLUDE_DIRECTORIES})
	endforeach()
	#target_sources(${PLUGIN_NAME} PRIVATE )
endif()

target_link_libraries(${PLUGIN_NAME} PUBLIC KlemmginePlugin)
target_include_directories(${PLUGIN_NAME} PRIVATE ${NET_SDK_BINARY_DIR})

if(WIN32)
	file(COPY "${NET_SDK_BINARY_DIR}/nethost.dll" DESTINATION "${KLEMMGINE_PLUGIN_DIR}")
	target_link_libraries(${PLUGIN_NAME} PRIVATE "${NET_SDK_BINARY_DIR}/nethost.lib")
else()
	file(COPY "${NET_SDK_BINARY_DIR}/libnethost.so" DESTINATION "${KLEMMGINE_PLUGIN_DIR}")
	target_link_libraries(${PLUGIN_NAME} PRIVATE "${KLEMMGINE_PLUGIN_DIR}/libnethost.so")
	target_link_options(${PLUGIN_NAME} PRIVATE "-Wl,-rpath,$ORIGIN")
endif()

add_dependencies(KlemmgineLib ${PLUGIN_NAME})