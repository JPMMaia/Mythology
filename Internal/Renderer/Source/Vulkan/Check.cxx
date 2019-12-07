export module maia.renderer.vulkan.check;

import <vulkan/vulkan.h>;

import <cassert>;

namespace Maia::Renderer::Vulkan
{
    export void check_result(VkResult const result) noexcept;
}


module: private;

namespace Maia::Renderer::Vulkan
{
    void check_result(VkResult const result) noexcept
    {
        assert(result == VK_SUCCESS);
    }
}