project (MaiaECSUnitTest)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../..//Utilities/CMake/Modules")

add_executable (MaiaECSUnitTest)
add_executable (Maia::GameEngine::UnitTest ALIAS MaiaECSUnitTest)

target_compile_features (MaiaECSUnitTest PRIVATE cxx_std_20)


add_module_compile_options (MaiaECSUnitTest)

find_package (Catch2 CONFIG REQUIRED)
target_link_libraries (MaiaECSUnitTest PRIVATE Catch2_header Catch2_runner)
add_module_dependency (MaiaECSUnitTest "catch2.catch")

target_include_directories(MaiaECSUnitTest PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries (MaiaECSUnitTest PRIVATE Maia::ECS Maia::ECS::Components Maia::ECS::Systems)

target_link_libraries (MaiaECSUnitTest PRIVATE Standard_library)

target_sources (MaiaECSUnitTest 
	PRIVATE
		"main.cpp"
		"Component_group.test.cpp"
		"Entity_manager.test.cpp"
		"Systems/Transform_system.test.cpp"
)

include (Catch)
catch_discover_tests (MaiaECSUnitTest)
