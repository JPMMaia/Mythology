module maia.renderer.vulkan.image;

import maia.renderer.vulkan.allocation_callbacks;
import maia.renderer.vulkan.check;
import maia.renderer.vulkan.device;

import <vulkan/vulkan.h>;

import <cstdint>;
import <optional>;
import <span>;

namespace Maia::Renderer::Vulkan
{
    Image create_image(
        Device const device,
        std::optional<Allocation_callbacks> const allocator,
        VkImageType const image_type,
        VkFormat const format,
        VkExtent3D const extent,
        Mip_level_count const mip_levels,
        Array_layer_count const array_layers,
        VkImageUsageFlags const usage,
        VkImageLayout const initial_layout,
        VkSampleCountFlagBits const samples,
        VkImageCreateFlags const flags,
        VkImageTiling const tiling,
        VkSharingMode const sharing_mode,
        std::span<std::uint32_t const> const queue_family_indices
    ) noexcept
    {
        VkImageCreateInfo const create_info
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = flags,
            .imageType = image_type,
            .format = format,
            .extent = extent,
            .mipLevels = mip_levels.value,
            .arrayLayers = array_layers.value,
            .samples = samples,
            .tiling = tiling,
            .usage = usage,
            .sharingMode = sharing_mode,
            .queueFamilyIndexCount = static_cast<uint32_t>(queue_family_indices.size()),
            .pQueueFamilyIndices = queue_family_indices.data(),
            .initialLayout = initial_layout,
        };

        VkImage image = {};
        check_result(
            vkCreateImage(
                device.value, 
                &create_info, 
                allocator.has_value() ? &allocator->value : nullptr, 
                &image
            )
        );

        return {image};
    }

    void destroy_image(
        Device const device,
        Image const image,
        std::optional<Allocation_callbacks> const allocator
    ) noexcept
    {
        vkDestroyImage(
            device.value,
            image.value,
            allocator.has_value() ? &allocator->value : nullptr
        );
    }
}