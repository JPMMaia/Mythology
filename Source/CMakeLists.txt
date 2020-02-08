project (MaiaMythology VERSION 0.0.1)

add_executable (MaiaMythology)
add_executable (Maia::Mythology ALIAS MaiaMythology)

set_target_properties (
	MaiaMythology
		PROPERTIES
			VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}"
)

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

add_module_compile_options (MaiaMythology)

find_package (docopt REQUIRED CONFIG)
target_link_libraries (MaiaMythology PRIVATE docopt)

target_link_libraries (MaiaMythology PRIVATE Standard_library)

target_link_libraries (MaiaMythology
	PRIVATE
		Maia::ECS
		Maia::Renderer
		#Maia::Resources
		#Maia::Shaders
		#Maia::Utilities
)


target_sources (MaiaMythology PRIVATE "main.cpp")

#[[
target_sources (MaiaMythology 
	PRIVATE
		"Application.hpp"
		"Application.cpp"

		"Camera.hpp"
		"IInput_system.hpp"
		"Input_state.hpp"
		"Input_state_views.hpp"
		"Input_state_views.cpp"
		"Game_key.hpp"
		"Game_clock.hpp"
		
		"Transform_freely_system.hpp"
		"Transform_freely_system.cpp"

		"Render/Pass_data.hpp"

		"Render/D3D12/Load_scene_system.hpp"
		"Render/D3D12/Load_scene_system.cpp"
		#"Render/D3D12/Scene.hpp"
		#"Render/D3D12/Scene.cpp"
		"Render/D3D12/Render_system.hpp"
		"Render/D3D12/Render_system.cpp"
		"Render/D3D12/Render_data.hpp"
		"Render/D3D12/Render_data.cpp"
		"Render/D3D12/Renderer.hpp"
		"Render/D3D12/Renderer.cpp"
		"Render/D3D12/Upload_frame_data_system.hpp"
		"Render/D3D12/Upload_frame_data_system.cpp"

		"Components/Camera_component.hpp"
		"Components/Camera_component.cpp"
		"Components/Mesh_ID.hpp"
		"Components/Mesh_ID.cpp"
)

if (WINDOWS_STORE)
	target_sources (MaiaMythology PRIVATE "Core_window_main.cpp")
else ()
	target_sources (
		MaiaMythology 
			PRIVATE 
				"Win32_main.cpp"
				"Win32/Window.hpp"
				"Win32/Window.cpp"
				"Win32/Input_system.hpp"
				"Win32/Input_system.cpp"
	)
endif()
]]#