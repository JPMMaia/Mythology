module maia.renderer.vulkan.command_pool;

import maia.renderer.vulkan.allocation_callbacks;
import maia.renderer.vulkan.check;
import maia.renderer.vulkan.device;

import <vulkan/vulkan.h>;

import <optional>;

namespace Maia::Renderer::Vulkan
{
    Command_pool create_command_pool(
        Device const device,
        VkCommandPoolCreateFlags const flags,
        Queue_family_index const queue_family_index,
        std::optional<Allocation_callbacks> const allocator
    ) noexcept
    {
        VkCommandPoolCreateInfo const create_info
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .queueFamilyIndex = {}
        };

        VkCommandPool command_pool = {};
        check_result(
            vkCreateCommandPool(
                device.value, 
                &create_info, 
                allocator.has_value() ? &allocator->value : nullptr,
                &command_pool
            )
        );

        return {command_pool};
    }

    void reset_command_pool(
        Device device,
        Command_pool command_pool,
        VkCommandPoolResetFlags flags
    ) noexcept
    {
        check_result(
            vkResetCommandPool(device.value, command_pool.value, flags));
    }

    void destroy_command_pool(
        Device device,
        Command_pool command_pool,
        std::optional<Allocation_callbacks> allocator
    ) noexcept
    {
        vkDestroyCommandPool(
            device.value, 
            command_pool.value,
            allocator.has_value() ? &allocator->value : nullptr
        );
    }
}