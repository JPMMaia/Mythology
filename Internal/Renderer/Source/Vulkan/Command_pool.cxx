export module maia.renderer.vulkan.command_pool;

import <vulkan/vulkan.h>;

import maia.renderer.vulkan.device;

namespace Maia::Renderer::Vulkan
{
    export VkCommandPool create_command_pool(
        VkDevice device,
        VkCommandPoolCreateFlags flags,
        Queue_family_index queue_family_index,
        VkAllocationCallbacks const* allocator
    ) noexcept;

    export void reset_command_pool(
        VkDevice device,
        VkCommandPool command_pool,
        VkCommandPoolResetFlags flags
    ) noexcept;

    export void destroy_command_pool(
        VkDevice device,
        VkCommandPool command_pool,
        VkAllocationCallbacks const* allocator
    ) noexcept;
}