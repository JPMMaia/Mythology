export module maia.renderer.vulkan.buffer;

import maia.renderer.vulkan.allocation_callbacks;
import maia.renderer.vulkan.device;

import <vulkan/vulkan.h>;

import <cstdint>;
import <optional>;
import <span>;

namespace Maia::Renderer::Vulkan
{
    export struct Buffer
    {
        VkBuffer value;
    };

    export Buffer create_buffer(
        Device device, 
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkBufferCreateFlags flags = {},
        VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE,
        std::span<std::uint32_t const> queue_family_indices = {},
        VkAllocationCallbacks const* allocator = nullptr
    ) noexcept;

    export void destroy_buffer(
        Device device,
        Buffer buffer,
        VkAllocationCallbacks const* allocator = nullptr
    ) noexcept;
}
