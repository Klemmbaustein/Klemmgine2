cmake_minimum_required(VERSION 3.12)

set(PLUGIN_NAME "Klemmgine.CSharp.Loader")

set(SRC_DIR "Loader")

include("FindSdk.cmake")

if(NOT NET_SDK_BINARY_DIR)
	message(".NET sdk not found, not compiling c# plugin...")
	return()
endif()

klemmgine_plugin_dirs()

option(USE_CSHARP_JIT "Use ahead of time C# binaries" "${ENGINE_EDITOR}")

set(CSHARP_DLL "${CMAKE_CURRENT_SOURCE_DIR}/bin/net8.0/Klemmgine.CSharp.dll")

file(
    GLOB_RECURSE
    CS_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Engine/**.cs"
	"${CMAKE_CURRENT_SOURCE_DIR}/Engine.Core/**.cs"
	"${CMAKE_CURRENT_SOURCE_DIR}/Engine.Aot/**.cs"
)

list(FILTER CS_SOURCES EXCLUDE REGEX "obj")

add_custom_command(
    OUTPUT ${CSHARP_DLL}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    DEPENDS ${CS_SOURCES}
    COMMAND dotnet build
)

add_custom_target(CSharpBinaries
    DEPENDS ${CS_SOURCES}
    DEPENDS ${CSHARP_DLL})

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

add_library(${PLUGIN_NAME} SHARED ${INCLUDES} ${SRCS} "Loader/Resources.rc")
# add_dependencies(${PLUGIN_NAME} CSharpBinaries)

if(NOT USE_CSHARP_JIT)
	target_compile_definitions(${PLUGIN_NAME} PRIVATE "USE_AOT_BINARIES")
endif()

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