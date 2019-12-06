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

target_module (
    MaiaRendererVulkan
    "maia.renderer.vulkan.allocation_callbacks"
    INTERFACE "Allocation_callbacks.ixx"
    INTERFACE_DEPENDS "Vulkan"
)

target_module (
    MaiaRendererVulkan
    "maia.renderer.vulkan.instance"
    INTERFACE "Instance.ixx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
    IMPLEMENTATION "Instance.mxx"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan"
)

target_module (
    MaiaRendererVulkan
    "maia.renderer.vulkan.physical_device"
    INTERFACE "Physical_device.ixx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
    IMPLEMENTATION "Physical_device.mxx"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan"
)

target_module (
    MaiaRendererVulkan
    "maia.renderer.vulkan.device"
    INTERFACE "Device.ixx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
    IMPLEMENTATION "Device.mxx"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan"
)

target_module (
    MaiaRendererVulkan
    "maia.renderer.vulkan.queue"
    INTERFACE "Queue.ixx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
    IMPLEMENTATION "Queue.mxx"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan"
)

target_module (
    MaiaRendererVulkan
    "maia.renderer.vulkan.device_memory"
    INTERFACE "Device_memory.ixx"
    INTERFACE_DEPENDS "Standard_library" "Vulkan"
    IMPLEMENTATION "Device_memory.mxx"
    IMPLEMENTATION_DEPENDS "Standard_library" "Vulkan"
)

target_module (
    MaiaRendererVulkan
    "maia.renderer.vulkan"
    INTERFACE "Module.ixx"
)
