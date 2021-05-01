add_executable (MaiaSceneUnitTest)
add_executable (Maia::Scene::UnitTest ALIAS MaiaSceneUnitTest)

target_compile_features (MaiaSceneUnitTest PRIVATE cxx_std_20)

find_package (Catch2 CONFIG REQUIRED)
target_link_libraries (MaiaSceneUnitTest PRIVATE Catch2_header Catch2_runner)
add_module_dependency (MaiaSceneUnitTest "catch2.catch")

add_module_compile_options (MaiaSceneUnitTest)

target_link_libraries (MaiaSceneUnitTest PRIVATE Maia::Scene Maia::Test)

target_sources (MaiaSceneUnitTest 
	PRIVATE
		"main.cpp"
)

include (Catch)
catch_discover_tests (MaiaSceneUnitTest)
