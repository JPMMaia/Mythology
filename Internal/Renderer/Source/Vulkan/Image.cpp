module maia.renderer.vulkan.image;

import maia.renderer.vulkan.allocation_callbacks;
import maia.renderer.vulkan.check;
import maia.renderer.vulkan.device;

import <vulkan/vulkan.h>;

import <cstdint>;
import <span>;

namespace Maia::Renderer::Vulkan
{
    Image create_image(
        Device const device,
        Allocation_callbacks const allocation_callbacks,
        VkImageType const image_type,
        VkFormat const format,
        VkExtent3D const extent,
        std::uint32_t const mip_levels,
        std::uint32_t const array_layers,
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
            .mipLevels = mip_levels,
            .arrayLayers = array_layers,
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
            vkCreateImage(device.value, &create_info, &allocation_callbacks.value, &image));

        return {image};
    }
}