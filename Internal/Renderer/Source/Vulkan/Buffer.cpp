module maia.renderer.vulkan.buffer;

import maia.renderer.vulkan.allocation_callbacks;
import maia.renderer.vulkan.check;
import maia.renderer.vulkan.device;

import <vulkan/vulkan.h>;

import <cstdint>;
import <span>;

namespace Maia::Renderer::Vulkan
{
    Buffer create_buffer(
        Device const device, 
        Allocation_callbacks const allocation_callbacks,
        VkBufferCreateFlags const flags,
        VkDeviceSize const size,
        VkBufferUsageFlags const usage,
        VkSharingMode const sharing_mode,
        std::span<std::uint32_t const> const queue_family_indices
    ) noexcept
    {
        VkBufferCreateInfo const info
        {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = flags,
            .size = size,
            .usage = usage,
            .sharingMode = sharing_mode,
            .queueFamilyIndexCount = static_cast<std::uint32_t>(queue_family_indices.size()),
            .pQueueFamilyIndices = queue_family_indices.data(),
        };

        VkBuffer buffer = {};
        check_result(
            vkCreateBuffer(device.value, &info, &allocation_callbacks.value, &buffer));
        
        return {buffer};
    }

    Buffer create_transfer_source_buffer(
        Device const device, 
        Allocation_callbacks const allocation_callbacks,
        VkDeviceSize const size,
        VkSharingMode const sharing_mode,
        std::span<std::uint32_t const> const queue_family_indices
    ) noexcept
    {
        return create_buffer(
            device, 
            allocation_callbacks, 
            {}, 
            size, 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            sharing_mode, 
            queue_family_indices
        );
    }

    Buffer create_non_mappable_vertex_buffer(
        Device const device, 
        Allocation_callbacks const allocation_callbacks,
        VkDeviceSize const size,
        VkSharingMode const sharing_mode,
        std::span<std::uint32_t const> const queue_family_indices
    ) noexcept
    {
        return create_buffer(
            device, 
            allocation_callbacks, 
            {}, 
            size, 
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            sharing_mode, 
            queue_family_indices
        );
    }

    Buffer create_non_mappable_index_buffer(
        Device const device, 
        Allocation_callbacks const allocation_callbacks,
        VkDeviceSize const size,
        VkSharingMode const sharing_mode,
        std::span<std::uint32_t const> const queue_family_indices
    ) noexcept
    {
        return create_buffer(
            device, 
            allocation_callbacks, 
            {}, 
            size, 
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            sharing_mode, 
            queue_family_indices
        );
    }

    Buffer create_non_mappable_uniform_buffer(
        Device const device, 
        Allocation_callbacks const allocation_callbacks,
        VkDeviceSize const size,
        VkSharingMode const sharing_mode,
        std::span<std::uint32_t const> const queue_family_indices
    ) noexcept
    {
        return create_buffer(
            device, 
            allocation_callbacks, 
            {}, 
            size, 
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            sharing_mode, 
            queue_family_indices
        );
    }

    Buffer create_non_mappable_storage_buffer(
        Device const device, 
        Allocation_callbacks const allocation_callbacks,
        VkDeviceSize const size,
        VkSharingMode const sharing_mode,
        std::span<std::uint32_t const> const queue_family_indices
    ) noexcept
    {
        return create_buffer(
            device, 
            allocation_callbacks, 
            {}, 
            size, 
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            sharing_mode, 
            queue_family_indices
        );
    }
}