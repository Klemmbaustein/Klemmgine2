# CMakeList.txt : CMake project for Klemmgine2, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)
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
option(ENGINE_SERVER "Is a server" OFF)
option(ENGINE_EDITOR "Compile the editor" OFF)
project("Klemmgine")

set(SRC_DIR "EngineSource")

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

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(USE_WINDOWS_SUBSYSTEM AND WIN32)
	add_executable(Klemmgine WIN32 ${SRCS} ${INCLUDES})
	target_compile_definitions(Klemmgine PUBLIC ENGINE_SUBSYSTEM_WIN32)
else()
	add_executable(Klemmgine ${SRCS} ${INCLUDES})
endif()

set(KLEMMUI_CUSTOM_SYSTEMWM ON)
set(KLEMMUI_INCLUDE_EXAMPLES OFF)

add_subdirectory("Dependencies/KlemmUI")
add_subdirectory("Dependencies/SDL")
set(BUILD_SHARED_LIBS OFF)
add_subdirectory("Dependencies/glm")

if(WIN32)
	target_compile_definitions(Klemmgine PUBLIC WINDOWS=1)
else()
	target_compile_definitions(Klemmgine PUBLIC LINUX=1)
endif()

if(USE_ENGINEMAIN)
	target_compile_definitions(Klemmgine PRIVATE USE_ENGINEMAIN)
endif()

if(ENGINE_SERVER)
	target_compile_definitions(Klemmgine PUBLIC SERVER)
elseif(ENGINE_EDITOR)
	set(ASSIMP_INSTALL OFF)
	set(ASSIMP_WARNINGS_AS_ERRORS OFF)
	set(ENABLE_ALL_WARNINGS OFF)
	set(BUILD_SHARED_LIBS OFF)
	set(ASSIMP_BUILD_TESTS OFF)
	set(ASSIMP_NO_EXPORT ON)
	SET(ASSIMP_INJECT_DEBUG_POSTFIX OFF)
	if (WIN32)
		# assimp causes hundreds of warnings with MSVC if this is on because they for some reason manually add
		# /Zi to generate debug files for debug builds.
		# (which then causes warnings because it conflicts with /ZI used for hotreloading above)
		set(OLD_MSVC_VALUE ${MSVC})
		set(MSVC OFF)

		# Add some other compile flags that cause warnings with MSVC
		add_compile_definitions("_CRT_SECURE_NO_WARNINGS" "_CRT_NONSTDC_NO_WARNINGS")
		add_compile_options(/wd4244)
	endif()
	add_subdirectory("Dependencies/assimp")
	set(MSVC ${OLD_MSVC_VALUE})

	if (WIN32)
		target_compile_options(Klemmgine PUBLIC "/w44244")
	endif()

	target_link_libraries(Klemmgine PUBLIC assimp::assimp)
	target_compile_definitions(Klemmgine PUBLIC EDITOR)
	klemmui_markup(Klemmgine "Engine/Editor/UI")
endif()

target_include_directories(Klemmgine PRIVATE "Dependencies/KlemmUI/Source/SystemWM")
target_include_directories(Klemmgine PRIVATE "EngineSource")
target_link_libraries(Klemmgine PUBLIC SDL3::SDL3)
target_link_libraries(Klemmgine PUBLIC KlemmUI)
target_link_libraries(Klemmgine PUBLIC libglew_static)
target_link_libraries(Klemmgine PUBLIC glm)

klemmui_resources(Klemmgine "Engine/Resources")

# TODO: Add tests and install targets if needed.
