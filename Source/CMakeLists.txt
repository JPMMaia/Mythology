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

add_subdirectory ("Core")
add_subdirectory ("ImGui")

add_subdirectory ("Windowless")
target_link_libraries (MaiaMythology PRIVATE Maia::Mythology::Windowless)

add_subdirectory ("Image")

if (MaiaMythology_use_SDL)

	add_subdirectory ("SDL")
	target_link_libraries (MaiaMythology PRIVATE Maia::Mythology::SDL)
	
endif()

target_link_libraries(MaiaMythology PRIVATE nlohmann_json_module)

target_sources (MaiaMythology PRIVATE "main.cpp")

install (
	TARGETS MaiaMythology
	RUNTIME
		DESTINATION "bin"
		COMPONENT "runtime"
)
