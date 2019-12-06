export module maia.renderer.vulkan.allocation_callbacks;

import <vulkan/vulkan.h>;

namespace Maia::Renderer::Vulkan
{
    export struct Allocation_callbacks
    {
        VkAllocationCallbacks value;
    };
}