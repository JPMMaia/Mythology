export module maia.renderer.vulkan.image;

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


    export VkImage create_image(
        VkDevice device,
        VkImageCreateInfo const& create_info,
        VkAllocationCallbacks const* allocator = nullptr
    ) noexcept;

    export VkImage create_image(
        VkDevice device,
        VkAllocationCallbacks const* allocator,
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

    export void destroy_image(
        VkDevice device,
        VkImage image,
        VkAllocationCallbacks const* allocator
    ) noexcept;

    
    export VkSubresourceLayout get_subresource_layout(
        VkDevice device,
        VkImage image,
        VkImageSubresource subresource
    ) noexcept;


    export struct Image_view
    {
        VkImageView value;
    };

    export struct Component_mapping
    {
        VkComponentMapping value = 
        {
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY
        };
    };

    export Image_view create_image_view(
        VkDevice device,
        VkImageViewCreateFlags flags,
        VkImage image,
        VkImageViewType view_type,
        VkFormat format,
        Component_mapping components,
        VkImageSubresourceRange subresource_range,
        VkAllocationCallbacks const* allocator
    ) noexcept;

    export void destroy_image_view(
        VkDevice device,
        Image_view image_view,
        VkAllocationCallbacks const* allocator
    ) noexcept;
}