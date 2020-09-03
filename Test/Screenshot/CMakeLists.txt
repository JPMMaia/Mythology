list(APPEND CMAKE_MODULE_PATH
	"${CMAKE_CURRENT_LIST_DIR}/../../Internal/Utilities/CMake/Modules"
)

add_executable (MythologyScreenshotTest)
add_executable (Mythology::ScreenshotTest ALIAS MythologyScreenshotTest)

target_compile_features (MythologyScreenshotTest PRIVATE cxx_std_20)


add_module_compile_options (MythologyScreenshotTest)

find_package (Catch2 CONFIG REQUIRED)
target_link_libraries (MythologyScreenshotTest PRIVATE Catch2_header Catch2_runner)
add_module_dependency (MythologyScreenshotTest "catch2.catch")

target_link_libraries (MythologyScreenshotTest PRIVATE nlohmann_json_module)
add_module_dependency (MythologyScreenshotTest "nlohmann_json")

target_include_directories(MythologyScreenshotTest PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries (MythologyScreenshotTest PRIVATE Maia::Mythology::Image)
target_link_libraries (MythologyScreenshotTest PRIVATE Maia::Mythology::Windowless)

target_link_libraries (MythologyScreenshotTest PRIVATE Standard_library)

target_sources (MythologyScreenshotTest 
	PRIVATE
		"main.cpp"
		"clear_color/Clear_color.test.cpp"
		"white_triangle/White_triangle.test.cpp"
)

install(
	FILES
		"clear_color/clear_blue_pipeline.json"
		
		"white_triangle/white_triangle_expected_output.png"
		"white_triangle/white_triangle_pipeline.json"
		"white_triangle/Triangle.vertex.spv"
		"white_triangle/White.fragment.spv"

	DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/test_assets"
	COMPONENT "test_assets"
)

add_custom_command(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/test_assets"
	COMMAND "${CMAKE_COMMAND}" "--install" "${CMAKE_BINARY_DIR}" "--component" "test_assets"
)

add_custom_target(
	install_test_assets
	DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/test_assets"
)

include (CTest)
include (Catch)
catch_discover_tests (MythologyScreenshotTest)
