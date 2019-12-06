add_library (MaiaRendererVulkan "Check.ixx")
add_library (Maia::Renderer::Vulkan ALIAS MaiaRendererVulkan)

set_target_properties (MaiaRendererVulkan PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries (MaiaRendererVulkan PRIVATE Vulkan)


target_module (
    MaiaRendererVulkan
    "maia.renderer.vulkan.check"
    INTERFACE "Check.ixx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
)

