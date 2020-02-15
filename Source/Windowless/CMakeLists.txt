add_library (MaiaMythologyWindowless)
add_library (Maia::Mythology::Windowless ALIAS MaiaMythologyWindowless)

target_link_libraries (MaiaMythologyWindowless PRIVATE Maia::Mythology::Core)

target_link_libraries (MaiaMythologyWindowless PRIVATE Standard_library)
target_link_libraries (MaiaMythologyWindowless PRIVATE Vulkan)

target_module (
    MaiaMythologyWindowless
    "mythology.windowless"
    INTERFACE "Windowless.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
    IMPLEMENTATION "Windowless.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan"
)
