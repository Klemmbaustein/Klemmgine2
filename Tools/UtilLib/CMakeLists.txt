cmake_minimum_required(VERSION 3.12)

set(SRC_DIR "../../EngineSource/Engine")

# Library that includes many of the utility functions of the engine.
add_library(EngineUtils STATIC 
	"${SRC_DIR}/Vector.cpp" "${SRC_DIR}/Transform.cpp" "${SRC_DIR}/StringUtil.cpp" "${SRC_DIR}/File/SerializedData.cpp"
	"${SRC_DIR}/Log.cpp" "${SRC_DIR}/ThreadPool.cpp" "${SRC_DIR}/LaunchArgs.cpp" "${SRC_DIR}/File/FileUtil.cpp"
	"${SRC_DIR}/File/TextSerializer.cpp" "${SRC_DIR}/File/BinarySerializer.cpp" "${SRC_DIR}/File/FileUtil.cpp"
	"${SRC_DIR}/Error/StackTrace.cpp" "${SRC_DIR}/Error/EngineError.cpp" "${SRC_DIR}/Error/EngineAssert.cpp"
	"${SRC_DIR}/Internal/Platform.cpp"
)

target_compile_definitions(EngineUtils PUBLIC ENGINE_UTILS_LIB=1)

if(WIN32)
	target_compile_definitions(EngineUtils PUBLIC WINDOWS=1)
else()
	target_compile_definitions(EngineUtils PUBLIC LINUX=1)
endif()

target_link_libraries(EngineUtils PUBLIC glm-header-only)
target_include_directories(EngineUtils PUBLIC "../../EngineSource/")