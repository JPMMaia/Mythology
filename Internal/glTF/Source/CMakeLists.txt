add_library (MaiaGlTF)
add_library (Maia::glTF ALIAS MaiaGlTF)

set_target_properties (MaiaGlTF PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(MaiaGlTF PRIVATE Maia::Scene)

find_package(nlohmann_json REQUIRED)
target_link_libraries(MaiaGlTF PRIVATE nlohmann_json::nlohmann_json)

target_module (
    MaiaGlTF
    PUBLIC
    "maia.glTF"
    INTERFACE "glTF.cppm"
    IMPLEMENTATION "glTF.cpp"
)
