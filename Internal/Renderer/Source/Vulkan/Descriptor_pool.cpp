module maia.renderer.vulkan.descriptor_pool;

import maia.renderer.vulkan.check;

import <vulkan/vulkan.h>;


namespace Maia::Renderer::Vulkan
{
    VkDescriptorPool create_descriptor_pool(
        VkDevice const device,
        VkDescriptorPoolCreateInfo const& create_info,
        VkAllocationCallbacks const* const allocator
    ) noexcept
    {
        VkDescriptorPool descriptor_pool{};

        check_result(
            vkCreateDescriptorPool(device, &create_info, allocator, &descriptor_pool));

        return descriptor_pool;
    }

    void destroy_descriptor_pool(
        VkDevice const device,
        VkDescriptorPool const descriptor_pool,
        VkAllocationCallbacks const* const allocator
    ) noexcept
    {
        vkDestroyDescriptorPool(
            device,
            descriptor_pool,
            allocator
        );
    }
}
