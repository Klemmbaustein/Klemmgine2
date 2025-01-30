# Why do I need to do "cmd /c" on windows?
if(WIN32)
	execute_process(COMMAND cmd /c dotnet --list-sdks OUTPUT_VARIABLE NET_SDKS)
else()
	execute_process(COMMAND dotnet --list-sdks OUTPUT_VARIABLE NET_SDKS RESULT_VARIABLE NET_FAILED)
endif()

if(NET_FAILED)
	message(WARNING "Failed to run dotnet")
	return()
endif()

string(REPLACE "\n" ";" NET_SDK_LIST "${NET_SDKS}")

set(LATEST_SDK_VERSION "8.0.0")

option(KLEMMGINE_USE_PREVIEW_NET_SDK "Allow using preview .NET sdks" OFF)

foreach(NET_SDK ${NET_SDK_LIST})
	# CMake sure is a language.
	string(FIND ${NET_SDK} " " FOUND_WHITESPACE)

	string(SUBSTRING ${NET_SDK} 0 ${FOUND_WHITESPACE} SDK_NAME)
	string(SUBSTRING ${NET_SDK} ${FOUND_WHITESPACE} -1 SDK_LOCATION)

	# Replace backslashes on windows for prettier output.
	if(WIN32)
		string(REPLACE "\\" "/" SDK_LOCATION ${SDK_LOCATION})
	endif()

	string(LENGTH ${SDK_LOCATION} SDK_LOCATION_LENGTH)
	math(EXPR SDK_LOCATION_LENGTH "${SDK_LOCATION_LENGTH} - 3")

	string(SUBSTRING ${SDK_LOCATION} 2 ${SDK_LOCATION_LENGTH} SDK_LOCATION)

	message(STATUS "Found .NET SDK path: ${SDK_LOCATION}/${SDK_NAME}")
	list(APPEND SDK_FULL_PATHS "${SDK_LOCATION}/${SDK_NAME}")

	if (KLEMMGINE_USE_PREVIEW_NET_SDK)
		if(${SDK_NAME} VERSION_GREATER "${LATEST_SDK_VERSION}")
			set(LATEST_SDK_VERSION "${SDK_NAME}")
			set(LATEST_SDK_PATH "${SDK_LOCATION}/${SDK_NAME}")
		endif()
	else()
		if(${SDK_NAME} VERSION_GREATER "${LATEST_SDK_VERSION}" AND NOT ${SDK_NAME} MATCHES "preview")
			set(LATEST_SDK_VERSION "${SDK_NAME}")
			set(LATEST_SDK_PATH "${SDK_LOCATION}/${SDK_NAME}")
		endif()
	endif()
endforeach()

if(NOT SDK_FULL_PATHS)
	message(FATAL_ERROR "Failed to find any .NET SDKs")
endif()

if(NOT LATEST_SDK_PATH)
	message(FATAL_ERROR "Failed to find any .NET SDKs later than .NET 8")
endif()

message("Using .NET SDK: ${LATEST_SDK_PATH}")

# dotnet/sdks/whatever -> dotnet/
set(NET_ROOT_PATH "${LATEST_SDK_PATH}/../..")

if(WIN32)
	if(CMAKE_SYSTEM_PROCESSOR STREQUAL "ARM64")
		set(NET_RUNTIME_ID "win-arm64")
	else()
		set(NET_RUNTIME_ID "win-x64")
	endif()
else()
	if(CMAKE_SYSTEM_PROCESSOR STREQUAL "ARM64")
		set(NET_RUNTIME_ID "linux-arm64")
	else()
		set(NET_RUNTIME_ID "linux-x64")
	endif()
endif()

set(PACK_PATH "${NET_ROOT_PATH}/packs/Microsoft.NETCore.App.Host.${NET_RUNTIME_ID}")

file(GLOB
	PACK_VERSIONS
	"${PACK_PATH}/*"
)

set(LATEST_PACK_PATH "0.0.0")

foreach(VER ${PACK_VERSIONS})
	get_filename_component(VERSION_NAME ${VER} NAME)

	if ("${VERSION_NAME}" VERSION_GREATER "${LATEST_PACK_PATH}")
		set(LATEST_PACK_PATH "${VER}")
	endif()
endforeach()

if ("${LATEST_PACK_PATH}" VERSION_EQUAL "0.0.0")
	return()
endif()

set(NET_SDK_BINARY_DIR "${LATEST_PACK_PATH}/runtimes/${NET_RUNTIME_ID}/native")
message(STATUS ".NET binary path: ${NET_SDK_BINARY_DIR}")
