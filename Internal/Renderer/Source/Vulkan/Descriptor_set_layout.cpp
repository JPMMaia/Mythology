module;

#include <vulkan/vulkan.h>

module maia.renderer.vulkan.descriptor_set_layout;

import maia.renderer.vulkan.check;
namespace Maia::Renderer::Vulkan
{
    VkDescriptorSetLayout create_descriptor_set_layout(
        VkDevice const device,
        VkDescriptorSetLayoutCreateInfo const& create_info,
        VkAllocationCallbacks const* const allocator
    ) noexcept
    {
        VkDescriptorSetLayout descriptor_set_layout{};

        check_result(
            vkCreateDescriptorSetLayout(device, &create_info, allocator, &descriptor_set_layout));

        return descriptor_set_layout;
    }

    void destroy_descriptor_set_layout(
        VkDevice const device,
        VkDescriptorSetLayout const descriptor_set_layout,
        VkAllocationCallbacks const* const allocator
    ) noexcept
    {
        vkDestroyDescriptorSetLayout(device, descriptor_set_layout, allocator);
    }
}
