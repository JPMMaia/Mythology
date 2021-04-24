project (MaiaECSUnitTest)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../..//Utilities/CMake/Modules")

add_library (MaiaTest)
add_library (Maia::Test ALIAS MaiaTest)

find_package (Catch2 CONFIG REQUIRED)
target_link_libraries (MaiaTest PUBLIC Catch2_header Catch2_runner)
add_module_dependency (MaiaTest "catch2.catch")

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

target_module (
    MaiaTest
    PUBLIC "maia.test.print_tuple"
    INTERFACE "Print_tuple.cxx"
)

add_subdirectory("Core")
