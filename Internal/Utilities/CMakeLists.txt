cmake_minimum_required(VERSION 3.12)

project (MaiaUtilities VERSION 0.0.1)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake/Modules")

add_subdirectory("External/Catch2")
add_subdirectory("External/Eigen3")
add_subdirectory("External/nlohmann_json")
#add_subdirectory("External/Standard_library")
add_subdirectory("External/Stb")

add_subdirectory("Source")
add_subdirectory("Test")
