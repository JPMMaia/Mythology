export module maia.renderer.vulkan.swapchain;

import maia.renderer.vulkan.allocation_callbacks;
import maia.renderer.vulkan.device;
import maia.renderer.vulkan.fence;
import maia.renderer.vulkan.image;
import maia.renderer.vulkan.queue;
import maia.renderer.vulkan.semaphore;
import maia.renderer.vulkan.surface;

import <vulkan/vulkan.h>;

import <cstdint>;
import <memory_resource>;
import <optional>;
import <span>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    export struct Swapchain
    {
        VkSwapchainKHR value = VK_NULL_HANDLE;
    };

    export struct Min_image_count
    {
        std::uint32_t value = 1;
    };

    export Swapchain create_swapchain(
        Device device,
        VkSwapchainCreateFlagsKHR flags,
        Min_image_count min_image_count,
        Surface surface,
        VkFormat image_format,
        VkColorSpaceKHR image_color_space,
        VkExtent2D image_extent,
        Array_layer_count image_array_layers,
        VkImageUsageFlags image_usage,
        VkSharingMode image_sharing_mode,
        std::span<Queue_family_index const> queue_family_indices,
        VkSurfaceTransformFlagBitsKHR pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VkPresentModeKHR present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR,
        bool clipped = true,
        Swapchain old_swapchain = {},
        std::optional<Allocation_callbacks> allocator = {}
    ) noexcept;

    export void destroy_swapchain(
        Device device,
        Swapchain swapchain,
        std::optional<Allocation_callbacks> allocator = {}
    ) noexcept;


    export std::pmr::vector<VkImage> get_swapchain_images(
        Device device,
        Swapchain swapchain,
        std::pmr::polymorphic_allocator<VkSurfaceFormatKHR> allocator = {}
    ) noexcept;


    export struct Swapchain_image_index
    {
        std::uint32_t value = 0;
    };

    export std::optional<Swapchain_image_index> acquire_next_image(
        Device device,
        Swapchain swapchain,
        std::uint64_t timeout,
        std::optional<Semaphore> semaphore,
        std::optional<Fence> fence
    ) noexcept;


    export void queue_present(
        Queue queue,
        std::span<VkSemaphore const> semaphores_to_wait,
        Swapchain swapchain,
        Swapchain_image_index swapchain_image_index
    ) noexcept;
}