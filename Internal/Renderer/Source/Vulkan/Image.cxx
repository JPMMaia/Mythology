export module maia.renderer.vulkan.image;

import maia.renderer.vulkan.allocation_callbacks;
import maia.renderer.vulkan.device;

import <vulkan/vulkan.h>;

import <cstdint>;
import <optional>;
import <span>;

namespace Maia::Renderer::Vulkan
{
    export struct Mip_level_count
    {
        std::uint32_t value;
    };

    export struct Array_layer_count
    {
        std::uint32_t value;
    };


    export struct Image
    {
        VkImage value;
    };

    export Image create_image(
        Device device,
        std::optional<Allocation_callbacks> allocator,
        VkImageType image_type,
        VkFormat format,
        VkExtent3D extent,
        Mip_level_count mip_levels,
        Array_layer_count array_layers,
        VkImageUsageFlags usage,
        VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED,
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
        VkImageCreateFlags flags = {},
        VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
        VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE,
        std::span<std::uint32_t const> queue_family_indices = {}
    ) noexcept;
}