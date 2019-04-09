cmake_minimum_required(VERSION 3.12)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")

project(MaiaUtilities LANGUAGES CXX)

add_subdirectory("Source")
add_subdirectory("Test")
