cmake_minimum_required(VERSION 3.12)

project(KlemmginePlugin)

option(PLUGIN_INTERFACE_CPP "C++ plugin interface" ON)

set(SRC_DIR "../../EngineSource/Core")

add_library(KlemmginePlugin 
	"Source/KlemmginePlugin.cpp"
)

target_link_libraries(KlemmginePlugin PUBLIC KlemmgineCore)

target_compile_definitions(KlemmginePlugin PUBLIC ENGINE_PLUGIN)
target_include_directories(KlemmginePlugin PUBLIC "../../EngineSource")

if(NOT WIN32)
	target_compile_options(KlemmginePlugin PUBLIC "-fPIC")
endif()

if(PLUGIN_INTERFACE_CPP)
	target_include_directories(KlemmginePlugin PUBLIC "Include/cpp")
else()
	target_include_directories(KlemmginePlugin PUBLIC "Include/c")
endif()

macro(klemmgine_plugin_dirs)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/plugins")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/plugins")
	set(KLEMMGINE_PLUGIN_DIR "${CMAKE_BINARY_DIR}/bin/plugins")
endmacro()
