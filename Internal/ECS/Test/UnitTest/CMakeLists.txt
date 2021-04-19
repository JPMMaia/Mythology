project (MaiaECSUnitTest)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../..//Utilities/CMake/Modules")

add_library (MaiaTest)
add_library (Maia::Test ALIAS MaiaTest)

set_target_properties(MaiaTest 
	PROPERTIES 
		LINKER_LANGUAGE CXX
)

target_module (
    MaiaTest
    PUBLIC "maia.test.debug_object"
    INTERFACE "Debug_object.cxx"
)

target_module (
    MaiaTest
    PUBLIC "maia.test.debug_resource"
    INTERFACE "Debug_resource.cxx"
)

# Unit tests

add_executable (MaiaECSUnitTest)
add_executable (Maia::ECS::UnitTest ALIAS MaiaECSUnitTest)

target_compile_definitions (MaiaECSUnitTest PRIVATE CATCH_CONFIG_ENABLE_BENCHMARKING)

target_compile_features (MaiaECSUnitTest PRIVATE cxx_std_20)


add_module_compile_options (MaiaECSUnitTest)

find_package (Catch2 CONFIG REQUIRED)
target_link_libraries (MaiaECSUnitTest PRIVATE Catch2_header Catch2_runner)
add_module_dependency (MaiaECSUnitTest "catch2.catch")

target_link_libraries (MaiaECSUnitTest PRIVATE Maia::ECS Maia::Test)

target_sources (MaiaECSUnitTest 
	PRIVATE
		"main.cpp"
		"Entity_manager_2.test.cpp"
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

target_link_libraries (MaiaECSConstexprTests PRIVATE Maia::ECS)

target_sources (MaiaECSConstexprTests 
	PRIVATE
		"main.cpp"
)

catch_discover_tests (MaiaECSConstexprTests)
