cmake_minimum_required(VERSION 3.12)

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")


include (FetchContent)

FetchContent_Declare (
	MaiaUtilities
	GIT_REPOSITORY "https://github.com/JPMMaia/Utilities.git"
	GIT_TAG "master"
)

FetchContent_GetProperties (MaiaUtilities)
if (NOT maiautilities_POPULATED)
	FetchContent_Populate (MaiaUtilities)
	add_subdirectory (${maiautilities_SOURCE_DIR} ${maiautilities_BINARY_DIR})
	list (APPEND CMAKE_MODULE_PATH "${maiautilities_SOURCE_DIR}/CMake")
endif ()


project(MaiaGameEngine LANGUAGES CXX)

add_subdirectory("Source")
add_subdirectory("Test")
