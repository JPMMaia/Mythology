add_library (MaiaScene)
add_library (Maia::Scene ALIAS MaiaScene)

set_target_properties (MaiaScene PROPERTIES LINKER_LANGUAGE CXX)

target_module (
    MaiaScene
    PUBLIC
    "maia.scene"
    INTERFACE "Scene.cxx"
    IMPLEMENTATION "Scene.cpp"
)
