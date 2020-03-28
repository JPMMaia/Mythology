add_library (MaiaMythologyCore)
add_library (Maia::Mythology::Core ALIAS MaiaMythologyCore)

target_link_libraries (MaiaMythologyCore PRIVATE Maia::Renderer::Vulkan)

target_link_libraries (MaiaMythologyCore PRIVATE Standard_library)
target_link_libraries (MaiaMythologyCore PRIVATE Vulkan)

target_module (
    MaiaMythologyCore
    "mythology.core.utilities"
    INTERFACE "Utilities.cxx"
    INTERFACE_DEPENDS "Standard_library"
)

target_module (
    MaiaMythologyCore
    "mythology.core.vulkan"
    INTERFACE "Vulkan.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan" "maia.renderer.vulkan"
    IMPLEMENTATION "Vulkan.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan" "maia.renderer.vulkan"
)
