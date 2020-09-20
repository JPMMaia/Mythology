add_library (MaiaRendererVulkan)
add_library (Maia::Renderer::Vulkan ALIAS MaiaRendererVulkan)

set_target_properties (MaiaRendererVulkan PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries (MaiaRendererVulkan PRIVATE Standard_library Vulkan)


target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.check"
    INTERFACE "Check.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.instance"
    INTERFACE "Instance.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
    IMPLEMENTATION "Instance.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan" "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.physical_device"
    INTERFACE "Physical_device.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
    IMPLEMENTATION "Physical_device.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan" "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.device"
    INTERFACE "Device.cxx"
    IMPLEMENTATION "Device.cpp"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.descriptor_set_layout"
    INTERFACE "Descriptor_set_layout.cxx"
    INTERFACE_DEPENDS "Vulkan"
    IMPLEMENTATION "Descriptor_set_layout.cpp"
    IMPLEMENTATION_DEPENDS "Vulkan" "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.descriptor_set"
    INTERFACE "Descriptor_set.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
    IMPLEMENTATION "Descriptor_set.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan" "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.fence"
    INTERFACE "Fence.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
    IMPLEMENTATION "Fence.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan"
        "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.semaphore"
    INTERFACE "Semaphore.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
    IMPLEMENTATION "Semaphore.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan"
        "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.queue"
    INTERFACE "Queue.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
        "maia.renderer.vulkan.check"
    IMPLEMENTATION "Queue.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.pipeline"
    INTERFACE "Pipeline.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
    IMPLEMENTATION "Pipeline.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan" 
        "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.pipeline_layout"
    INTERFACE "Pipeline_layout.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
    IMPLEMENTATION "Pipeline_layout.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan" 
        "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.buffer"
    INTERFACE "Buffer.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
    IMPLEMENTATION "Buffer.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan"
        "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.image"
    INTERFACE "Image.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
    IMPLEMENTATION "Image.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan"
        "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.device_memory"
    INTERFACE "Device_memory.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
        "maia.renderer.vulkan.buffer"
        "maia.renderer.vulkan.image"
    IMPLEMENTATION "Device_memory.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan" 
        "maia.renderer.vulkan.buffer"
        "maia.renderer.vulkan.check"
        "maia.renderer.vulkan.image"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.render_pass"
    INTERFACE "Render_pass.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
    IMPLEMENTATION "Render_pass.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan" 
        "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.shader_module"
    INTERFACE "Shader_module.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
    IMPLEMENTATION "Shader_module.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan" 
        "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.command_pool"
    INTERFACE "Command_pool.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan" "maia.renderer.vulkan.device"
    IMPLEMENTATION "Command_pool.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan" 
        "maia.renderer.vulkan.check"
        "maia.renderer.vulkan.device"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.command_buffer"
    INTERFACE "Command_buffer.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan" "maia.renderer.vulkan.device"
    IMPLEMENTATION "Command_buffer.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan" 
        "maia.renderer.vulkan.check"
        "maia.renderer.vulkan.device"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.sampler"
    INTERFACE "Sampler.cxx"
    INTERFACE_DEPENDS "Vulkan"
    IMPLEMENTATION "Sampler.cpp"
    IMPLEMENTATION_DEPENDS "Vulkan" 
        "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.surface"
    INTERFACE "Surface.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
    IMPLEMENTATION "Surface.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan" 
        "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.swapchain"
    INTERFACE "Swapchain.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
    IMPLEMENTATION "Swapchain.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan" 
        "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.memory_management"
    INTERFACE "Memory_management.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
    IMPLEMENTATION "Memory_management.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan"
        "maia.renderer.vulkan.buffer"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.descriptor_pool"
    INTERFACE "Descriptor_pool.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
    IMPLEMENTATION "Descriptor_pool.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan" "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan"
    INTERFACE "Module.cxx"
    INTERFACE_DEPENDS
        "maia.renderer.vulkan.buffer"
        "maia.renderer.vulkan.check"
        "maia.renderer.vulkan.command_buffer"
        "maia.renderer.vulkan.command_pool"
        "maia.renderer.vulkan.descriptor_pool"
        "maia.renderer.vulkan.descriptor_set_layout"
        "maia.renderer.vulkan.descriptor_set"
        "maia.renderer.vulkan.device"
        "maia.renderer.vulkan.device_memory"
        "maia.renderer.vulkan.fence"
        "maia.renderer.vulkan.image"
        "maia.renderer.vulkan.instance"
        "maia.renderer.vulkan.memory_management"
        "maia.renderer.vulkan.physical_device"
        "maia.renderer.vulkan.pipeline"
        "maia.renderer.vulkan.pipeline_layout"
        "maia.renderer.vulkan.queue"
        "maia.renderer.vulkan.render_pass"
        "maia.renderer.vulkan.sampler"
        "maia.renderer.vulkan.semaphore"
        "maia.renderer.vulkan.shader_module"
        "maia.renderer.vulkan.surface"
        "maia.renderer.vulkan.swapchain"
)





add_library (MaiaRendererVulkanSerialization)
add_library (Maia::Renderer::Vulkan::Serialization ALIAS MaiaRendererVulkanSerialization)

set_target_properties (MaiaRendererVulkanSerialization PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries (MaiaRendererVulkanSerialization PRIVATE Vulkan MaiaRendererVulkan)

find_package(nlohmann_json REQUIRED)
target_link_libraries (MaiaRendererVulkanSerialization PRIVATE nlohmann_json_module)

target_module (
    MaiaRendererVulkanSerialization
    PUBLIC
    "maia.renderer.vulkan.serializer"
    INTERFACE "serialization/Serializer.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan" "nlohmann_json_module"
    IMPLEMENTATION "serialization/Serializer.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan" "nlohmann_json_module"
)
