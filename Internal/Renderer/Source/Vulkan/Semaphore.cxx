export module maia.renderer.vulkan.semaphore;

import <vulkan/vulkan.h>;

namespace Maia::Renderer::Vulkan
{
    export struct Semaphore
    {
        VkSemaphore value;
    };
}
