project (MaiaECSUnitTest)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../..//Utilities/CMake/Modules")

# Unit tests

add_executable (MaiaECSUnitTest)
add_executable (Maia::ECS::UnitTest ALIAS MaiaECSUnitTest)

target_compile_definitions (MaiaECSUnitTest PRIVATE CATCH_CONFIG_ENABLE_BENCHMARKING)

target_compile_features (MaiaECSUnitTest PRIVATE cxx_std_20)


add_module_compile_options (MaiaECSUnitTest)

find_package (Catch2 CONFIG REQUIRED)
target_link_libraries (MaiaECSUnitTest PRIVATE Catch2_header Catch2_runner)
add_module_dependency (MaiaECSUnitTest "catch2.catch")

target_link_libraries (MaiaECSUnitTest PRIVATE Maia::ECS Maia::ECS::Components)
target_link_libraries (MaiaECSUnitTest PRIVATE Standard_library)

target_sources (MaiaECSUnitTest 
	PRIVATE
		"main.cpp"
		"Entity_manager.test.cpp"
		"Component_chunk_group.test.cpp"
		"Component_chunk.test.cpp"
		#"Archetype.test.cpp"
		#"Component.test.cpp"
		#"Component_group.test.cpp"
		#"Entity_manager.test.cpp"
		#"Systems/Transform_system.test.cpp"
)

include (Catch)
catch_discover_tests (MaiaECSUnitTest)


# Constexpr tests

add_executable (MaiaECSConstexprTests)
add_executable (Maia::ECS::ConstexprTests ALIAS MaiaECSConstexprTests)

target_compile_features (MaiaECSConstexprTests PRIVATE cxx_std_20)


add_module_compile_options (MaiaECSConstexprTests)

find_package (Catch2 CONFIG REQUIRED)
target_link_libraries (MaiaECSConstexprTests PRIVATE Catch2_header Catch2_runner)
add_module_dependency (MaiaECSConstexprTests "catch2.catch")

target_link_libraries (MaiaECSConstexprTests PRIVATE Maia::ECS Maia::ECS::Components)
target_link_libraries (MaiaECSConstexprTests PRIVATE Standard_library)

target_sources (MaiaECSConstexprTests 
	PRIVATE
		"main.cpp"
		"Archetype.constexpr.test.cpp"
)

catch_discover_tests (MaiaECSConstexprTests)
