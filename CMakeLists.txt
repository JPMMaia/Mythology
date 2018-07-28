cmake_minimum_required(VERSION 3.11)
project(Mythology)

add_executable(MythologyUWP)
add_executable(Mythology::UWP ALIAS MythologyUWP)

target_sources(MythologyUWP 
    PRIVATE
		"Source/App.cpp"
		"Source/D3D12_renderer.h"
		"Source/D3D12_renderer.cpp"
)

target_include_directories(MythologyUWP PRIVATE "Source")

target_compile_features(MythologyUWP PRIVATE cxx_std_17)

if (MSVC)
	target_compile_options(MythologyUWP PRIVATE "/permissive-")
endif ()

list (APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/..")

find_package (Eigen3 3.3 REQUIRED NO_MODULE)

add_subdirectory("${MAIA_GAME_ENGINE_ROOT}" "${CMAKE_CURRENT_SOURCE_DIR}/Binaries/Maia/GameEngine")
target_link_libraries(MythologyUWP PRIVATE Maia::GameEngine)
target_include_directories(MythologyUWP PRIVATE "${MAIA_GAME_ENGINE_ROOT}/Source")

add_subdirectory("${MAIA_RENDERER_ROOT}" "${CMAKE_CURRENT_SOURCE_DIR}/Binaries/Maia/Renderer")
target_link_libraries(MythologyUWP PRIVATE Maia::Renderer)
target_include_directories(MythologyUWP PRIVATE "${MAIA_RENDERER_ROOT}/Source")

add_subdirectory("${MAIA_UTILITIES_ROOT}" "${CMAKE_CURRENT_SOURCE_DIR}/Binaries/Maia/Utilities")
target_link_libraries(MythologyUWP INTERFACE Maia::Utilities)
target_include_directories(MythologyUWP PRIVATE "${MAIA_UTILITIES_ROOT}/Source")
