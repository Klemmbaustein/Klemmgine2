cmake_minimum_required(VERSION 3.12)

set(SRC_DIR "../../EngineSource/Engine")

# Library that includes many of the utility functions of the engine.
add_library(EngineUtils STATIC 
	"${SRC_DIR}/Vector.cpp" "${SRC_DIR}/Transform.cpp" "${SRC_DIR}/StringUtil.cpp" "${SRC_DIR}/File/SerializedData.cpp"
	"${SRC_DIR}/Log.cpp" "${SRC_DIR}/ThreadPool.cpp" "${SRC_DIR}/LaunchArgs.cpp" "${SRC_DIR}/File/FileUtil.cpp"
	"${SRC_DIR}/File/TextSerializer.cpp" "${SRC_DIR}/File/BinarySerializer.cpp" "${SRC_DIR}/File/FileUtil.cpp"
	"${SRC_DIR}/Error/StackTrace.cpp" "${SRC_DIR}/Error/EngineError.cpp" "${SRC_DIR}/Error/EngineAssert.cpp"
)

target_compile_definitions(EngineUtils PUBLIC ENGINE_UTILS_LIB=1)