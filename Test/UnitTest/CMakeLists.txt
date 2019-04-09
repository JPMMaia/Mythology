project (MaiaRendererUnitTest)

add_executable (MaiaRendererUnitTest)
add_executable (Maia::Renderer::UnitTest ALIAS MaiaRendererUnitTest)

target_compile_features (MaiaRendererUnitTest PRIVATE cxx_std_17)

target_link_libraries (MaiaRendererUnitTest PRIVATE Maia::Renderer)

find_package (Catch2 REQUIRED)
target_link_libraries (MaiaRendererUnitTest PRIVATE Catch2::Catch2)

target_sources (MaiaRendererUnitTest 
	PRIVATE
		"main.cpp"
		"Matrices.test.cpp"
		#"D3D12/Utilities/D3D12_utilities_test.cpp"
)

include (Catch)
catch_discover_tests (MaiaRendererUnitTest)
