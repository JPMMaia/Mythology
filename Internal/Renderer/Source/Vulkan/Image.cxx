export module maia.renderer.vulkan.image;

import maia.renderer.vulkan.allocation_callbacks;
import maia.renderer.vulkan.device;

import <vulkan/vulkan.h>;

import <cstdint>;
import <span>;

namespace Maia::Renderer::Vulkan
{
    export struct Image
    {
        VkImage value;
    };

    export Image create_image(
        Device device,
        Allocation_callbacks allocation_callbacks,
        VkImageCreateFlags flags,
        VkImageType image_type,
        VkFormat format,
        VkExtent3D extent,
        std::uint32_t mip_levels,
        std::uint32_t array_layers,
        VkSampleCountFlagBits samples,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkSharingMode sharing_mode,
        std::span<std::uint32_t const> queue_family_indices,
        VkImageLayout initial_layout
    ) noexcept;
}