list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../..//Utilities/CMake/Modules")

add_executable (MaiaRendererUnitTest)
add_executable (Maia::Renderer::UnitTest ALIAS MaiaRendererUnitTest)

add_module_compile_options (MaiaRendererUnitTest)

target_link_libraries (MaiaRendererUnitTest PRIVATE Catch2_header Catch2_runner)
add_module_dependency (MaiaRendererUnitTest "catch2.catch")

target_link_libraries (MaiaRendererUnitTest PRIVATE Maia::Renderer::Vulkan)

target_link_libraries (MaiaRendererUnitTest PRIVATE Standard_library)

target_sources (MaiaRendererUnitTest 
	PRIVATE
		"main.cpp"
		"Vulkan/Initialize_test.cpp"
		#"Matrices.test.cpp"
		#"D3D12/Utilities/D3D12_utilities_test.cpp"
)
