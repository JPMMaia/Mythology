module;

#include <vulkan/vulkan.h>

module maia.renderer.vulkan.sampler;

import maia.renderer.vulkan.check;
namespace Maia::Renderer::Vulkan
{
    VkSampler create_sampler(
        VkDevice const device,
        VkSamplerCreateInfo const& create_info,
        VkAllocationCallbacks const* const allocator
    ) noexcept
    {
        VkSampler sampler{};

        check_result(
            vkCreateSampler(device, &create_info, allocator, &sampler));

        return sampler;
    }

    void destroy_sampler(
        VkDevice const device,
        VkSampler const sampler,
        VkAllocationCallbacks const* const allocator
    ) noexcept
    {
        vkDestroySampler(device, sampler, allocator);
    }
}
