cmake_minimum_required(VERSION 3.12)

project(Mythology)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Libraries")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Binaries")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Binaries")

include("Internal/Utilities/CMake/Scripts/modules.cmake")

add_subdirectory("Internal/Utilities")
add_subdirectory("Internal/Renderer")
add_subdirectory("Internal/ECS")
#add_subdirectory("Internal/Shaders")
#add_subdirectory("Internal/Resources")
#add_subdirectory("Source")
