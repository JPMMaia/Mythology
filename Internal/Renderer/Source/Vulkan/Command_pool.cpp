module maia.renderer.vulkan.command_pool;

import maia.renderer.vulkan.check;

import <vulkan/vulkan.h>;

import <optional>;

namespace Maia::Renderer::Vulkan
{
    VkCommandPool create_command_pool(
        VkDevice const device,
        VkCommandPoolCreateFlags const flags,
        Queue_family_index const queue_family_index,
        VkAllocationCallbacks const* const allocator
    ) noexcept
    {
        VkCommandPoolCreateInfo const create_info
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = flags,
            .queueFamilyIndex = queue_family_index.value
        };

        VkCommandPool command_pool = {};
        check_result(
            vkCreateCommandPool(
                device, 
                &create_info, 
                allocator,
                &command_pool
            )
        );

        return {command_pool};
    }

    void reset_command_pool(
        VkDevice device,
        VkCommandPool command_pool,
        VkCommandPoolResetFlags flags
    ) noexcept
    {
        check_result(
            vkResetCommandPool(device, command_pool, flags));
    }

    void destroy_command_pool(
        VkDevice device,
        VkCommandPool command_pool,
        VkAllocationCallbacks const* allocator
    ) noexcept
    {
        vkDestroyCommandPool(
            device, 
            command_pool,
            allocator
        );
    }
}