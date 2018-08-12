cmake_minimum_required(VERSION 3.12)

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")


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


FetchContent_Declare (
	MaiaGameEngine
	GIT_REPOSITORY "https://github.com/JPMMaia/GameEngine.git"
	GIT_TAG "master"
)

FetchContent_GetProperties (MaiaGameEngine)
if (NOT maiagameengine_POPULATED)
	FetchContent_Populate (MaiaGameEngine)
	add_subdirectory (${maiagameengine_SOURCE_DIR} ${maiagameengine_BINARY_DIR})
endif ()


project (Mythology)

add_subdirectory ("Source")
