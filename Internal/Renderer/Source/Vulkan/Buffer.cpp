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
    Buffer create_buffer(
        Device const device, 
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

        VkBuffer buffer = {};
        check_result(
            vkCreateBuffer(
                device.value, 
                &info, 
                allocator,
                &buffer
            )
        );
        
        return {buffer};
    }

    void destroy_buffer(
        Device const device,
        Buffer const buffer,
        VkAllocationCallbacks const* const allocator
    ) noexcept
    {
        vkDestroyBuffer(
            device.value,
            buffer.value,
            allocator
        );
    }
}