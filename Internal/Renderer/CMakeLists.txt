cmake_minimum_required (VERSION 3.12)

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake/Modules")

if (MaiaRenderer_build_vulkan)
    add_subdirectory ("External/Vulkan")
endif ()

add_subdirectory ("Source")
add_subdirectory ("Test")
