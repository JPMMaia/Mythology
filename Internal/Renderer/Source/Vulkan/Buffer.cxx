export module maia.renderer.vulkan.buffer;

import maia.renderer.vulkan.allocation_callbacks;
import maia.renderer.vulkan.device;

import <vulkan/vulkan.h>;

import <cstdint>;
import <span>;

namespace Maia::Renderer::Vulkan
{
    export struct Buffer
    {
        VkBuffer value;
    };

    export Buffer create_buffer(
        Device device, 
        Allocation_callbacks allocation_callbacks,
        VkBufferCreateFlags flags,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE,
        std::span<std::uint32_t const> queue_family_indices = {}
    ) noexcept;

    export Buffer create_transfer_source_buffer(
        Device device, 
        Allocation_callbacks allocation_callbacks,
        VkDeviceSize size,
        VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE,
        std::span<std::uint32_t const> queue_family_indices = {}
    ) noexcept;

    export Buffer create_non_mappable_vertex_buffer(
        Device device, 
        Allocation_callbacks allocation_callbacks,
        VkDeviceSize size,
        VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE,
        std::span<std::uint32_t const> queue_family_indices = {}
    ) noexcept;

    export Buffer create_non_mappable_index_buffer(
        Device device, 
        Allocation_callbacks allocation_callbacks,
        VkDeviceSize size,
        VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE,
        std::span<std::uint32_t const> queue_family_indices = {}
    ) noexcept;

    export Buffer create_non_mappable_uniform_buffer(
        Device device, 
        Allocation_callbacks allocation_callbacks,
        VkDeviceSize size,
        VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE,
        std::span<std::uint32_t const> queue_family_indices = {}
    ) noexcept;

    export Buffer create_non_mappable_storage_buffer(
        Device device, 
        Allocation_callbacks allocation_callbacks,
        VkDeviceSize size,
        VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE,
        std::span<std::uint32_t const> queue_family_indices = {}
    ) noexcept;
}
