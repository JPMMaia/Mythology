module;

#include <vulkan/vulkan.h>

export module maia.renderer.vulkan.sampler;
namespace Maia::Renderer::Vulkan
{
    export VkSampler create_sampler(
        VkDevice device,
        VkSamplerCreateInfo const& create_info,
        VkAllocationCallbacks const* allocator = nullptr
    ) noexcept;

    export void destroy_sampler(
        VkDevice device,
        VkSampler sampler,
        VkAllocationCallbacks const* allocator = nullptr
    ) noexcept;
}
