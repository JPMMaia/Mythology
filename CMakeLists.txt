cmake_minimum_required(VERSION 3.12)

project(Mythology)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Libraries")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Binaries")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Binaries")

include("Internal/Utilities/CMake/Scripts/modules.cmake")

if (MSVC)
if (WIN32)
    add_compile_definitions("NOMINMAX")
endif ()


add_subdirectory("External")

enable_testing()

add_subdirectory("Internal/Utilities")
add_subdirectory("Internal/Renderer")
add_subdirectory("Internal/ECS")
add_subdirectory("Internal/Input")
add_subdirectory("Internal/Shaders")
add_subdirectory("Internal/Scene")
add_subdirectory("Internal/glTF")
#add_subdirectory("Internal/Resources")
add_subdirectory("Source")
add_subdirectory("Test")

add_subdirectory("Addons/Example")