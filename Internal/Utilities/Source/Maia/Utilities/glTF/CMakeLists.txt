add_library (MaiaUtilitiesGLTF)
add_library (Maia::Utilities::GLTF ALIAS MaiaUtilitiesGLTF)

set_target_properties (MaiaUtilitiesGLTF PROPERTIES LINKER_LANGUAGE CXX)

find_package(nlohmann_json REQUIRED)
target_link_libraries (MaiaUtilitiesGLTF PRIVATE nlohmann_json_module)

target_module (
    MaiaUtilitiesGLTF
    PUBLIC
    "maia.utilities.gltf"
    INTERFACE "gltf.cxx"
    INTERFACE_DEPENDS "Standard_library" "nlohmann_json_module"
    IMPLEMENTATION "gltf.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "nlohmann_json_module"
)
