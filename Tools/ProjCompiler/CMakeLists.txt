cmake_minimum_required(VERSION 3.12)

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

add_executable(ProjectCompiler ${SRCS} ${INCLUDES})

add_dependencies(ProjectCompiler Klemmgine)

target_link_libraries(ProjectCompiler PRIVATE KlemmgineCore)