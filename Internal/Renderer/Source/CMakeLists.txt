project (MaiaRenderer VERSION 0.0.1)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../Utilities/CMake/Modules")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../Utilities/CMake/Scripts")

add_library (MaiaRenderer)
add_library (Maia::Renderer ALIAS MaiaRenderer)

set_target_properties (MaiaRenderer PROPERTIES 
	LINKER_LANGUAGE "CXX"
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
)

target_link_libraries (MaiaRenderer PRIVATE Maia::Utilities)

if (MaiaRenderer_build_d3d12)
	add_subdirectory ("D3D12")
	target_link_libraries (MaiaRenderer PRIVATE Maia::Renderer::D3D12)
endif ()

if (MaiaRenderer_build_vulkan)
	add_subdirectory ("Vulkan")
	target_link_libraries (MaiaRenderer PRIVATE Maia::Renderer::Vulkan)
endif ()

find_package (Eigen3 3.3.7 CONFIG REQUIRED)
target_link_libraries (MaiaRenderer PUBLIC Eigen3::Eigen)

find_package (MicrosoftGSL REQUIRED)
target_link_libraries (MaiaRenderer PRIVATE MicrosoftGSL::MicrosoftGSL)

target_sources (MaiaRenderer 
	PRIVATE 
		"Matrices.hpp"
		"Matrices.cpp"
)
