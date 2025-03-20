add_library(miniz STATIC "${CMAKE_CURRENT_LIST_DIR}/miniz/miniz.c")
target_include_directories(miniz PUBLIC "${CMAKE_CURRENT_LIST_DIR}/miniz")