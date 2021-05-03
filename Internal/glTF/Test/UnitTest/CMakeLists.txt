add_executable (MaiaGlTFUnitTest)
add_executable (Maia::glTF::UnitTest ALIAS MaiaGlTFUnitTest)

target_compile_features (MaiaGlTFUnitTest PRIVATE cxx_std_20)

find_package (Catch2 CONFIG REQUIRED)
target_link_libraries (MaiaGlTFUnitTest PRIVATE Catch2_header Catch2_runner)
add_module_dependency (MaiaGlTFUnitTest "catch2.catch")

add_module_compile_options (MaiaGlTFUnitTest)

target_link_libraries (MaiaGlTFUnitTest PRIVATE Maia::glTF Maia::Test)

target_sources (MaiaGlTFUnitTest 
	PRIVATE
		"main.cpp"
)

include (Catch)
catch_discover_tests (MaiaGlTFUnitTest)
