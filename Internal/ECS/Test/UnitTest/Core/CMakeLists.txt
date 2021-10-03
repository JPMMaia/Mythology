add_executable (MaiaECSUnitTest)
add_executable (Maia::ECS::UnitTest ALIAS MaiaECSUnitTest)

target_compile_definitions (MaiaECSUnitTest PRIVATE CATCH_CONFIG_ENABLE_BENCHMARKING)

add_module_compile_options (MaiaECSUnitTest)

target_link_libraries (MaiaECSUnitTest PRIVATE Maia::ECS Maia::Test)

target_sources (MaiaECSUnitTest 
	PRIVATE
		"main.cpp"
		"Component_groups.test.cpp"
		"Entity_manager.test.cpp"
)

include (Catch)
catch_discover_tests (MaiaECSUnitTest)
