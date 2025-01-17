cmake_minimum_required(VERSION 3.12)

project(KlemmginePlugin)

option(PLUGIN_INTERFACE_CPP "C++ plugin interface" ON)

set(SRC_DIR "../../EngineSource/Engine")

add_library(KlemmginePlugin 
	"${SRC_DIR}/Vector.cpp" "${SRC_DIR}/Transform.cpp" "${SRC_DIR}/StringUtil.cpp" "${SRC_DIR}/File/SerializedData.cpp"
	"Source/KlemmginePlugin.cpp")

target_compile_definitions(KlemmginePlugin PUBLIC ENGINE_PLUGIN)
target_include_directories(KlemmginePlugin PUBLIC "../../EngineSource")

if(NOT WIN32)
	target_compile_options(KlemmginePlugin PUBLIC "-fPIC")
endif()

target_link_libraries(KlemmginePlugin PUBLIC glm-header-only)

if(PLUGIN_INTERFACE_CPP)
	target_include_directories(KlemmginePlugin PUBLIC "Include/cpp")
else()
	target_include_directories(KlemmginePlugin PUBLIC "Include/c")
endif()
