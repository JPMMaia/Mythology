project (MaiaMythology VERSION 0.0.1)

add_executable (MaiaMythology)
add_executable (Maia::Mythology ALIAS MaiaMythology)

target_compile_features (MaiaMythology 
	PRIVATE 
		cxx_std_17
)

if (MSVC)
	target_compile_options (MaiaMythology 
		PRIVATE 
			"/permissive-"
	)
endif ()

if (WIN32)
	target_compile_definitions (MaiaMythology
		PRIVATE
			NOMINMAX
	)
endif ()

target_include_directories (MaiaMythology 
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
)

target_link_libraries (MaiaMythology
	PRIVATE
		Maia::GameEngine
		Maia::Renderer
		Maia::Utilities
)

target_sources (MaiaMythology 
	PRIVATE
		"App.cpp"
		"Scene.hpp"
		"Scene.cpp"
		"Renderer/D3D12/Renderer.hpp"
		"Renderer/D3D12/Renderer.cpp"
		"Renderer/D3D12/Render_data.hpp"
		"Renderer/D3D12/Render_data.cpp"

		"Renderer/Shaders/Color_vertex_shader.hlsl"
		"Renderer/Shaders/Color_pixel_shader.hlsl"
)

include ("${maiarenderer_SOURCE_DIR}/CMake/Scripts/target_add_hlsl_shader.cmake")

target_add_hlsl_shader (
	MaiaMythology
		"Shaders"
		"${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Shaders/Color_vertex_shader.hlsl"
		"${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/AppX/Resources/Shaders/Color_vertex_shader.csv"
		"main" "vs_5_1" true ""
)
target_add_hlsl_shader (
	MaiaMythology
		"Shaders"
		"${CMAKE_CURRENT_SOURCE_DIR}/Renderer/Shaders/Color_pixel_shader.hlsl"
		"${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/AppX/Resources/Shaders/Color_pixel_shader.csv"
		"main" "ps_5_1" true ""
)
