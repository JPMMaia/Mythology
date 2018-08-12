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
)
