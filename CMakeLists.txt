# CMakeList.txt : CMake project for Klemmgine2, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.12)

if(WIN32)
	set(CMAKE_CXX_STANDARD 23)
else()
	set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_C_STANDARD 17)

# Enable Hot Reload for MSVC compilers if supported.
if(POLICY CMP0141)
	cmake_policy(SET CMP0141 NEW)
	set(
		CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
		"$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>"
	)
endif()

option(USE_ENGINEMAIN "Use EngineMain() function as a replacement for main() or WinMain()" ON)
option(USE_WINDOWS_SUBSYSTEM "Use the Windows subsystem on windows. When this is off, a console will be allocated for the program." ON)
option(ENGINE_DYNAMIC_SYMBOLS "Link with -rdynamic on linux for more descriptive stack traces" OFF)
option(ENGINE_SERVER "Is a server" OFF)
option(ENGINE_EDITOR "Compile the editor" OFF)
option(ENGINE_CSHARP "Include the C# plugin" ON)
option(ENGINE_BUILD_TOOLS "Include all tools in the Tools/ directory" ON)

if(WIN32)
	project("Klemmgine" LANGUAGES C CXX RC)
else()
	project("Klemmgine" LANGUAGES C CXX)
endif()

if(MSVC)
	# Allow multi processor compilation with MSVC
	if(${CMAKE_GENERATOR} STREQUAL "Visual Studio 17 2022")
		add_compile_options("/MP")
	endif()
	add_compile_options("/FS")
endif()

function(engine_part_library TARGET_NAME TARGET_PATH)
	message("Including engine part: EngineSource/${TARGET_PATH} (${TARGET_NAME})")

	set(SRC_DIR "EngineSource")

	file(
		GLOB_RECURSE
		SRCS
		"${SRC_DIR}/${TARGET_PATH}/**.cpp"
	)
	file(
		GLOB_RECURSE
		INCLUDES
		"${SRC_DIR}/${TARGET_PATH}/**.h"
	)

	add_library(${TARGET_NAME} STATIC ${SRCS} ${INCLUDES})
endfunction()

if(UNIX)
	add_compile_options("-fPIC")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib$<0:>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin$<0:>)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin$<0:>)

if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)
	set(INTERPROCEDURAL_OPTIMIZATION OFF)
endif()

engine_part_library(KlemmgineCore "Core")
engine_part_library(KlemmgineLib "Engine")
if(ENGINE_EDITOR)
	engine_part_library(KlemmgineEditor "Editor")
	target_link_libraries(KlemmgineEditor PUBLIC KlemmgineLib)
endif()

target_compile_definitions(KlemmgineCore PRIVATE ENGINE_CORE)
target_link_libraries(KlemmgineLib PUBLIC KlemmgineCore)

if(USE_WINDOWS_SUBSYSTEM AND WIN32)
	add_executable(Klemmgine WIN32 "EngineSource/main.cpp" "EngineSource/Resources.rc")
	target_compile_definitions(KlemmgineLib PUBLIC ENGINE_SUBSYSTEM_WIN32)
else()
	add_executable(Klemmgine "EngineSource/main.cpp" "EngineSource/Resources.rc")
endif()

target_compile_definitions(KlemmgineLib PRIVATE "ENGINE_COMPILER_ID=${CMAKE_CXX_COMPILER_ID} ${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_BUILD_TYPE}")

set(KLEMMUI_CUSTOM_SYSTEMWM ON)
set(KLEMMUI_INCLUDE_EXAMPLES OFF)

if(UNIX)
	if(ENGINE_DYNAMIC_SYMBOLS)
		target_link_options(Klemmgine PUBLIC "-rdynamic")
	endif()
endif()

include("Dependencies/miniz.cmake")

set(KLEMMUI_DYNAMIC_MARKUP ON)
add_subdirectory("Dependencies/KlemmUI" EXCLUDE_FROM_ALL)
set(ENABLE_ALL_WARNINGS OFF)
set(GENERATE_DEBUG_SYMBOLS OFF)
set(CPP_RTTI_ENABLED ON)
set(OVERRIDE_CXX_FLAGS OFF)
add_subdirectory("Dependencies/SDL" EXCLUDE_FROM_ALL)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory("Dependencies/glm" EXCLUDE_FROM_ALL)
set(DEBUG_RENDERER_IN_DEBUG_AND_RELEASE OFF)
set(PROFILER_IN_DEBUG_AND_RELEASE OFF)
add_subdirectory("Dependencies/JoltPhysics/Build" EXCLUDE_FROM_ALL)
target_include_directories(KlemmgineLib PRIVATE "Dependencies/JoltPhysics/")
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
	set(GENERATE_DEBUG_SYMBOLS ON)
endif()

target_include_directories(KlemmgineLib PRIVATE "Plugins/PluginInterface/Include/internal")
target_compile_definitions(KlemmgineLib PRIVATE BUILD_LIBRARY=1)

if(WIN32)
	target_compile_definitions(KlemmgineCore PUBLIC WINDOWS=1)
else()
	target_compile_definitions(KlemmgineCore PUBLIC LINUX=1)
endif()

if(USE_ENGINEMAIN)
	target_compile_definitions(KlemmgineLib PRIVATE USE_ENGINEMAIN)
endif()

if(ENGINE_SERVER)
	target_compile_definitions(KlemmgineLib PUBLIC SERVER)
elseif(ENGINE_EDITOR)
	set(ASSIMP_INSTALL OFF)
	set(ASSIMP_WARNINGS_AS_ERRORS OFF)
	set(BUILD_SHARED_LIBS ON)
	set(ASSIMP_BUILD_TESTS OFF)
	set(ASSIMP_NO_EXPORT ON)
	set(ASSIMP_BUILD_ZLIB ON)
	SET(ASSIMP_INJECT_DEBUG_POSTFIX OFF)
	if(MSVC AND NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
		set(USING_MSVC ON)
	else()
		set(USING_MSVC OFF)
	endif()

	if (WIN32 AND USING_MSVC)
		# assimp causes hundreds of warnings with MSVC if this is on because they for some reason manually add
		# /Zi to generate debug files for debug builds.
		# (which then causes warnings because it conflicts with /ZI used for hotreloading above)
		set(MSVC OFF)

		# Add some other compile flags that cause warnings with MSVC
		add_compile_definitions("_CRT_SECURE_NO_WARNINGS" "_CRT_NONSTDC_NO_WARNINGS")
		add_compile_options(/wd4244)
	endif()
	add_subdirectory("Dependencies/assimp" EXCLUDE_FROM_ALL)

	if (WIN32 AND USING_MSVC)
		set(MSVC ON)
		target_compile_options(KlemmgineLib PUBLIC "/w44244")
	endif()

	target_link_libraries(KlemmgineLib PUBLIC assimp::assimp)
	target_compile_definitions(KlemmgineLib PUBLIC EDITOR)
	set(BUILD_SHARED_LIBS OFF)
	klemmui_markup(KlemmgineLib "Engine/Editor/UI")
endif()

# KlemmUI SystemWM declarations
target_include_directories(KlemmgineLib PRIVATE "Dependencies/KlemmUI/Source/SystemWM")
# KlemmUI stb headers
target_include_directories(KlemmgineLib PRIVATE "Dependencies/KlemmUI/Source/Util")
target_include_directories(KlemmgineCore PUBLIC "EngineSource")
target_link_libraries(KlemmgineLib PUBLIC SDL3::SDL3)
target_link_libraries(KlemmgineLib PUBLIC Jolt)
target_link_libraries(KlemmgineLib PUBLIC KlemmUI)
target_link_libraries(KlemmgineLib PUBLIC KuiDynamicMarkup)
target_link_libraries(KlemmgineLib PUBLIC libglew_static)
target_link_libraries(KlemmgineCore PUBLIC glm)
target_link_libraries(KlemmgineCore PUBLIC miniz)

target_link_libraries(Klemmgine PUBLIC KlemmgineLib KlemmUI KlemmgineLib)

if(NOT "${CXX_COMPILER_ID}" STREQUAL "MSVC" AND ENGINE_EDITOR)
	target_link_libraries(Klemmgine PUBLIC KlemmgineEditor)
endif()

if(UNIX)
	target_link_options(Klemmgine PUBLIC "-Wl,-rpath,$ORIGIN")
endif()


klemmui_resources(KlemmgineLib "Engine/Resources")


if (ENGINE_BUILD_TOOLS)
	enable_testing()
	add_subdirectory("Tools/Tests")
	add_subdirectory("Tools/ProjCompiler")
endif()

if(ENGINE_CSHARP)
	add_subdirectory("Plugins/PluginInterface")
	add_subdirectory("Plugins/CSharp")
	add_subdirectory("Plugins/DebugUI")
endif()

# TODO: Add tests and install targets if needed.
