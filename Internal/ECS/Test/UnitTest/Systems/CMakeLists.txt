add_executable (MaiaECSSystemsUnitTest)
add_executable (Maia::ECS::Systems::UnitTest ALIAS MaiaECSSystemsUnitTest)

target_compile_definitions (MaiaECSSystemsUnitTest PRIVATE CATCH_CONFIG_ENABLE_BENCHMARKING)

target_compile_features (MaiaECSSystemsUnitTest PRIVATE cxx_std_20)


add_module_compile_options (MaiaECSSystemsUnitTest)

target_link_libraries (MaiaECSSystemsUnitTest PRIVATE Maia::ECS::Systems Maia::Test)

find_package(klein 2.3.0 REQUIRED)
target_link_libraries(MaiaECSSystemsUnitTest PUBLIC klein::klein_sse42)

target_sources (MaiaECSSystemsUnitTest 
	PRIVATE
		"main.cpp"
		"Transform_system.test.cpp"
)

include (Catch)
catch_discover_tests (MaiaECSSystemsUnitTest)
