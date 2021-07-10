add_library (MaiaMythologyWindowless)
add_library (Maia::Mythology::Windowless ALIAS MaiaMythologyWindowless)

target_link_libraries (MaiaMythologyWindowless PRIVATE Maia::Renderer::Vulkan)
target_link_libraries (MaiaMythologyWindowless PRIVATE Maia::Mythology::Core)

target_link_libraries (MaiaMythologyWindowless PRIVATE nlohmann_json_module)
target_link_libraries (MaiaMythologyWindowless PRIVATE Standard_library)
target_link_libraries (MaiaMythologyWindowless PRIVATE Vulkan)

target_module (
    MaiaMythologyWindowless
    PUBLIC
    "mythology.windowless"
    INTERFACE "Windowless.cxx"
    INTERFACE_DEPENDS "nlohmann_json_module" "Standard_library" "Vulkan"
    IMPLEMENTATION "Windowless.cpp"
    IMPLEMENTATION_DEPENDS "nlohmann_json_module" "Standard_library" "Vulkan"
)
