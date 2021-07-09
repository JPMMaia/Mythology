add_library (MaiaRendererVulkan)
add_library (Maia::Renderer::Vulkan ALIAS MaiaRendererVulkan)

set_target_properties (MaiaRendererVulkan PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(MaiaRendererVulkan PUBLIC Vulkan)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.check"
    INTERFACE "Check.cxx"
    IMPLEMENTATION "Check.cpp"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.instance"
    INTERFACE "Instance.cxx"
    IMPLEMENTATION "Instance.cpp"
    IMPLEMENTATION_DEPENDS "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.physical_device"
    INTERFACE "Physical_device.cxx"
    IMPLEMENTATION "Physical_device.cpp"
    IMPLEMENTATION_DEPENDS "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.device"
    INTERFACE "Device.cxx"
    INTERFACE_DEPENDS "maia.renderer.vulkan.check"
    IMPLEMENTATION "Device.cpp"
    IMPLEMENTATION_DEPENDS "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.descriptor_set_layout"
    INTERFACE "Descriptor_set_layout.cxx"
    IMPLEMENTATION "Descriptor_set_layout.cpp"
    IMPLEMENTATION_DEPENDS "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.descriptor_set"
    INTERFACE "Descriptor_set.cxx"
    IMPLEMENTATION "Descriptor_set.cpp"
    IMPLEMENTATION_DEPENDS "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.fence"
    INTERFACE "Fence.cxx"
    IMPLEMENTATION "Fence.cpp"
    IMPLEMENTATION_DEPENDS
        "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.semaphore"
    INTERFACE "Semaphore.cxx"
    IMPLEMENTATION "Semaphore.cpp"
    IMPLEMENTATION_DEPENDS
        "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.queue"
    INTERFACE "Queue.cxx"
    INTERFACE_DEPENDS
        "maia.renderer.vulkan.device"
    IMPLEMENTATION "Queue.cpp"
    IMPLEMENTATION_DEPENDS
        "maia.renderer.vulkan.check"
        "maia.renderer.vulkan.device"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.pipeline"
    INTERFACE "Pipeline.cxx"
    IMPLEMENTATION "Pipeline.cpp"
    IMPLEMENTATION_DEPENDS 
        "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.pipeline_layout"
    INTERFACE "Pipeline_layout.cxx"
    IMPLEMENTATION "Pipeline_layout.cpp"
    IMPLEMENTATION_DEPENDS 
        "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.buffer"
    INTERFACE "Buffer.cxx"
    IMPLEMENTATION "Buffer.cpp"
    IMPLEMENTATION_DEPENDS
        "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.image"
    INTERFACE "Image.cxx"
    IMPLEMENTATION "Image.cpp"
    IMPLEMENTATION_DEPENDS
        "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.device_memory"
    INTERFACE "Device_memory.cxx"
    INTERFACE_DEPENDS
        "maia.renderer.vulkan.buffer"
        "maia.renderer.vulkan.image"
    IMPLEMENTATION "Device_memory.cpp"
    IMPLEMENTATION_DEPENDS 
        "maia.renderer.vulkan.buffer"
        "maia.renderer.vulkan.check"
        "maia.renderer.vulkan.image"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.render_pass"
    INTERFACE "Render_pass.cxx"
    IMPLEMENTATION "Render_pass.cpp"
    IMPLEMENTATION_DEPENDS 
        "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.shader_module"
    INTERFACE "Shader_module.cxx"
    IMPLEMENTATION "Shader_module.cpp"
    IMPLEMENTATION_DEPENDS 
        "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.command_pool"
    INTERFACE "Command_pool.cxx"
    INTERFACE_DEPENDS "maia.renderer.vulkan.device"
    IMPLEMENTATION "Command_pool.cpp"
    IMPLEMENTATION_DEPENDS 
        "maia.renderer.vulkan.check"
        "maia.renderer.vulkan.device"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.command_buffer"
    INTERFACE "Command_buffer.cxx"
    INTERFACE_DEPENDS "maia.renderer.vulkan.device"
    IMPLEMENTATION "Command_buffer.cpp"
    IMPLEMENTATION_DEPENDS 
        "maia.renderer.vulkan.check"
        "maia.renderer.vulkan.device"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.sampler"
    INTERFACE "Sampler.cxx"
    IMPLEMENTATION "Sampler.cpp"
    IMPLEMENTATION_DEPENDS 
        "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.surface"
    INTERFACE "Surface.cxx"
    INTERFACE_DEPENDS
        "maia.renderer.vulkan.device"
    IMPLEMENTATION "Surface.cpp"
    IMPLEMENTATION_DEPENDS 
        "maia.renderer.vulkan.check"
        "maia.renderer.vulkan.device"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.swapchain"
    INTERFACE "Swapchain.cxx"
    INTERFACE_DEPENDS
        "maia.renderer.vulkan.device"
        "maia.renderer.vulkan.image"
    IMPLEMENTATION "Swapchain.cpp"
    IMPLEMENTATION_DEPENDS 
        "maia.renderer.vulkan.check"
        "maia.renderer.vulkan.device"
        "maia.renderer.vulkan.image"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.memory_management"
    INTERFACE "Memory_management.cxx"
    INTERFACE_DEPENDS "maia.renderer.vulkan.device_memory"
    IMPLEMENTATION "Memory_management.cpp"
    IMPLEMENTATION_DEPENDS "maia.renderer.vulkan.device_memory"
        "maia.renderer.vulkan.buffer"
)

target_module (
    MaiaRendererVulkan
    PUBLIC
    "maia.renderer.vulkan.descriptor_pool"
    INTERFACE "Descriptor_pool.cxx"
    IMPLEMENTATION "Descriptor_pool.cpp"
    IMPLEMENTATION_DEPENDS "maia.renderer.vulkan.check"
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

target_link_libraries(MaiaRendererVulkanSerialization PUBLIC Vulkan)

find_package(nlohmann_json REQUIRED)
target_link_libraries (MaiaRendererVulkanSerialization PUBLIC nlohmann_json::nlohmann_json)

target_module (
    MaiaRendererVulkanSerialization
    PUBLIC
    "maia.renderer.vulkan.serializer"
    INTERFACE "serialization/Serializer.cxx"
    IMPLEMENTATION "serialization/Serializer.cpp"
)
