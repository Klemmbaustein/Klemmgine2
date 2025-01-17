cmake_minimum_required(VERSION 3.12)

add_subdirectory("../PluginInterface" "PluginInterface")

add_library(TestPlugin SHARED "Plugin.cpp")
target_link_libraries(TestPlugin PRIVATE KlemmginePlugin)