add_library (MaiaRendererVulkan)
add_library (Maia::Renderer::Vulkan ALIAS MaiaRendererVulkan)

set_target_properties (MaiaRendererVulkan PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(MaiaRendererVulkan PUBLIC Vulkan)

find_package(nlohmann_json REQUIRED)
target_link_libraries (MaiaRendererVulkan PUBLIC nlohmann_json::nlohmann_json)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.buffer_resources"
    INTERFACE "Buffer_resources.cxx"
    IMPLEMENTATION "Buffer_resources.cpp"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.serializer"
    INTERFACE "Serializer.cxx"
    IMPLEMENTATION "Serializer.cpp"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.upload"
    INTERFACE "Upload.cxx"
    INTERFACE_DEPENDS "maia.renderer.vulkan.buffer_resources"
    IMPLEMENTATION "Upload.cpp"
    IMPLEMENTATION_DEPENDS "maia.renderer.vulkan.buffer_resources"
)
