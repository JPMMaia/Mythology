export module maia.renderer.vulkan.fence;

import <vulkan/vulkan.h>;

namespace Maia::Renderer::Vulkan
{
    export struct Fence
    {
        VkFence value;
    };
}
