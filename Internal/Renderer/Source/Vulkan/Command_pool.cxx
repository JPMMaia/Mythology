export module maia.renderer.vulkan.command_pool;

import maia.renderer.vulkan.device;

import <vulkan/vulkan.h>;

import <optional>;

namespace Maia::Renderer::Vulkan
{
    export struct Command_pool
    {
        VkCommandPool value = VK_NULL_HANDLE;
    };

    export Command_pool create_command_pool(
        Device device,
        VkCommandPoolCreateFlags flags,
        Queue_family_index queue_family_index,
        VkAllocationCallbacks const* allocator
    ) noexcept;

    export void reset_command_pool(
        Device device,
        Command_pool command_pool,
        VkCommandPoolResetFlags flags
    ) noexcept;

    export void destroy_command_pool(
        Device device,
        Command_pool command_pool,
        VkAllocationCallbacks const* allocator
    ) noexcept;
}