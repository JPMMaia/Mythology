cmake_minimum_required(VERSION 3.12)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")

add_subdirectory("Internal/Utilities")
add_subdirectory("Internal/Renderer")
add_subdirectory("Internal/GameEngine")
add_subdirectory("Source")
