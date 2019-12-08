add_library (MaiaRendererVulkan)
add_library (Maia::Renderer::Vulkan ALIAS MaiaRendererVulkan)

set_target_properties (MaiaRendererVulkan PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries (MaiaRendererVulkan PRIVATE Vulkan)


target_module (
    MaiaRendererVulkan
    "maia.renderer.vulkan.check"
    INTERFACE "Check.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
)

target_module (
    MaiaRendererVulkan
    "maia.renderer.vulkan.allocation_callbacks"
    INTERFACE "Allocation_callbacks.cxx"
    INTERFACE_DEPENDS "Vulkan"
)

target_module (
    MaiaRendererVulkan
    "maia.renderer.vulkan.instance"
    INTERFACE "Instance.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
    IMPLEMENTATION "Instance.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan" "maia.renderer.vulkan.check"
)

target_module (
    MaiaRendererVulkan
    "maia.renderer.vulkan.physical_device"
    INTERFACE "Physical_device.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan" "maia.renderer.vulkan.instance"
    IMPLEMENTATION "Physical_device.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan" "maia.renderer.vulkan.check" "maia.renderer.vulkan.instance"
)

target_module (
    MaiaRendererVulkan
    "maia.renderer.vulkan.device"
    INTERFACE "Device.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan" "maia.renderer.vulkan.physical_device"
    IMPLEMENTATION "Device.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan" "maia.renderer.vulkan.check" "maia.renderer.vulkan.physical_device"
)

target_module (
    MaiaRendererVulkan
    "maia.renderer.vulkan.queue"
    INTERFACE "Queue.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
    IMPLEMENTATION "Queue.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan"
)

target_module (
    MaiaRendererVulkan
    "maia.renderer.vulkan.buffer"
    INTERFACE "Buffer.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
        "maia.renderer.vulkan.allocation_callbacks"
        "maia.renderer.vulkan.device"
    IMPLEMENTATION "Buffer.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan"
        "maia.renderer.vulkan.allocation_callbacks"
        "maia.renderer.vulkan.check"
        "maia.renderer.vulkan.device"
)

target_module (
    MaiaRendererVulkan
    "maia.renderer.vulkan.image"
    INTERFACE "Image.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
        "maia.renderer.vulkan.allocation_callbacks"
        "maia.renderer.vulkan.device"
    IMPLEMENTATION "Image.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan"
        "maia.renderer.vulkan.allocation_callbacks"
        "maia.renderer.vulkan.check"
        "maia.renderer.vulkan.device"
)

target_module (
    MaiaRendererVulkan
    "maia.renderer.vulkan.device_memory"
    INTERFACE "Device_memory.cxx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
        "maia.renderer.vulkan.allocation_callbacks"
        "maia.renderer.vulkan.buffer"
        "maia.renderer.vulkan.device"
        "maia.renderer.vulkan.image"
        "maia.renderer.vulkan.physical_device"
    IMPLEMENTATION "Device_memory.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan" 
        "maia.renderer.vulkan.allocation_callbacks"
        "maia.renderer.vulkan.buffer"
        "maia.renderer.vulkan.check"
        "maia.renderer.vulkan.device"
        "maia.renderer.vulkan.image"
        "maia.renderer.vulkan.physical_device"
)

target_module (
    MaiaRendererVulkan
    "maia.renderer.vulkan"
    INTERFACE "Module.cxx"
    INTERFACE_DEPENDS
        "maia.renderer.vulkan.check"
        "maia.renderer.vulkan.device"
        "maia.renderer.vulkan.instance"
        "maia.renderer.vulkan.physical_device"
)
