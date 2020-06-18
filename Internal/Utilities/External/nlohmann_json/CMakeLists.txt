find_package (nlohmann_json REQUIRED)
get_target_property(nlohmann_json_include_directories nlohmann_json::nlohmann_json INTERFACE_INCLUDE_DIRECTORIES)

add_library (nlohmann_json_module)

target_include_directories (nlohmann_json_module PUBLIC "${nlohmann_json_include_directories}")

set_target_properties (nlohmann_json_module PROPERTIES LINKER_LANGUAGE CXX)

list (GET nlohmann_json_include_directories 0 nlohmann_json_include_directory)

target_header (nlohmann_json_module "nlohmann_json" "${nlohmann_json_include_directory}/nlohmann/json.hpp")
