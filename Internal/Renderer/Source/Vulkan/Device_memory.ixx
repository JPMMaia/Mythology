export module maia.renderer.vulkan.device_memory;

import <vulkan/vulkan.h>;

namespace Maia::Renderer::Vulkan
{
    export struct Device_memory
    {
        VkDeviceMemory value;
    };
}