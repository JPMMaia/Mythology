module;

#include <vulkan/vulkan.h>

#include <cstdint>
#include <optional>
#include <span>

export module maia.renderer.vulkan.buffer;

namespace Maia::Renderer::Vulkan
{
    export VkBuffer create_buffer(
        VkDevice device, 
        VkBufferCreateInfo const& create_info,
        VkAllocationCallbacks const* allocator = nullptr
    ) noexcept;

    export VkBuffer create_buffer(
        VkDevice device, 
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkBufferCreateFlags flags = {},
        VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE,
        std::span<std::uint32_t const> queue_family_indices = {},
        VkAllocationCallbacks const* allocator = nullptr
    ) noexcept;


    export void destroy_buffer(
        VkDevice device,
        VkBuffer buffer,
        VkAllocationCallbacks const* allocator = nullptr
    ) noexcept;
}
