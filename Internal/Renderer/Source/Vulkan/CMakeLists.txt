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
    INTERFACE "Buffer_resources.cppm"
    IMPLEMENTATION "Buffer_resources.cpp"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.image_resources"
    INTERFACE "Image_resources.cppm"
    IMPLEMENTATION "Image_resources.cpp"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.upload"
    INTERFACE "Upload.cppm"
    INTERFACE_DEPENDS "maia.renderer.vulkan.buffer_resources"
    IMPLEMENTATION "Upload.cpp"
    IMPLEMENTATION_DEPENDS "maia.renderer.vulkan.buffer_resources"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.serializer"
    INTERFACE "Serializer.cppm"
    INTERFACE_DEPENDS "maia.renderer.vulkan.buffer_resources" "maia.renderer.vulkan.image_resources"
    IMPLEMENTATION "Serializer.cpp"
    IMPLEMENTATION_DEPENDS "maia.renderer.vulkan.buffer_resources" "maia.renderer.vulkan.image_resources" "maia.renderer.vulkan.upload"
)
