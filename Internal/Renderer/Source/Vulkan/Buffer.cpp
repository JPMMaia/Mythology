module maia.renderer.vulkan.buffer;

import maia.renderer.vulkan.allocation_callbacks;
import maia.renderer.vulkan.check;
import maia.renderer.vulkan.device;

import <vulkan/vulkan.h>;

import <cstdint>;
import <optional>;
import <span>;

namespace Maia::Renderer::Vulkan
{
    VkBuffer create_buffer(
        VkDevice const device, 
        VkBufferCreateInfo const& create_info,
        VkAllocationCallbacks const* const allocator
    ) noexcept
    {
        VkBuffer buffer = {};

        check_result(
            vkCreateBuffer(
                device, 
                &create_info, 
                allocator,
                &buffer
            )
        );
        
        return buffer;
    }

    VkBuffer create_buffer(
        VkDevice const device, 
        VkDeviceSize const size,
        VkBufferUsageFlags const usage,
        VkBufferCreateFlags const flags,
        VkSharingMode const sharing_mode,
        std::span<std::uint32_t const> const queue_family_indices,
        VkAllocationCallbacks const* const allocator
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

        return create_buffer(device, info, allocator);
    }

    void destroy_buffer(
        VkDevice const device,
        VkBuffer const buffer,
        VkAllocationCallbacks const* const allocator
    ) noexcept
    {
        vkDestroyBuffer(
            device,
            buffer,
            allocator
        );
    }
}