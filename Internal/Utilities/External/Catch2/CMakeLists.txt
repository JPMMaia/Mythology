find_package (Catch2 CONFIG REQUIRED)
get_target_property(Catch2_include_directories Catch2::Catch2 INTERFACE_INCLUDE_DIRECTORIES)
find_path(Catch2_include_directory "catch2/catch.hpp" PATHS ${Catch2_include_directories})


add_library (Catch2_header)

set_target_properties (Catch2_header PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries (Catch2_header PRIVATE Catch2::Catch2)

target_header (Catch2_header PUBLIC "catch2.catch" "${Catch2_include_directory}/catch2/catch.hpp")


add_library (Catch2_runner)

target_compile_definitions (Catch2_runner PUBLIC CATCH_CONFIG_ENABLE_BENCHMARKING)

target_link_libraries (Catch2_runner PUBLIC Catch2::Catch2)

set_target_properties (Catch2_runner PROPERTIES LINKER_LANGUAGE CXX)

target_module(Catch2_runner 
    PUBLIC
    "catch2.runner"
    INTERFACE "runner.cppm"
    IMPLEMENTATION "runner.cpp"
)
