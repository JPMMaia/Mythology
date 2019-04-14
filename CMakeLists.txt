cmake_minimum_required(VERSION 3.12)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Libraries")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Binaries")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Binaries")

add_subdirectory("Internal/Utilities")
add_subdirectory("Internal/Renderer")
add_subdirectory("Internal/GameEngine")
add_subdirectory("Internal/Shaders")
add_subdirectory("Internal/Resources")
add_subdirectory("Source")
