cmake_minimum_required(VERSION 3.12)

set(PLUGIN_NAME "Klemmgine.DebugUI")

set(SRC_DIR "Source")

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

add_library(${PLUGIN_NAME} SHARED ${INCLUDES} ${SRCS} "Source/Resources.rc")

if(WIN32)
	set(CMAKE_RC_STANDARD_INCLUDE_DIRECTORIES ${CUSTOM_INCLUDE_DIRECTORIES})
endif()

target_link_libraries(${PLUGIN_NAME} PUBLIC KlemmginePlugin)

add_dependencies(KlemmgineLib ${PLUGIN_NAME})