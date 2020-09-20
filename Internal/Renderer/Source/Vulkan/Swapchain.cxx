export module maia.renderer.vulkan.swapchain;


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
        VkDevice device,
        VkSwapchainCreateFlagsKHR flags,
        Min_image_count min_image_count,
        VkSurfaceKHR surface,
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
        VkAllocationCallbacks const* allocator = {}
    ) noexcept;

    export void destroy_swapchain(
        VkDevice device,
        Swapchain swapchain,
        VkAllocationCallbacks const* allocator = {}
    ) noexcept;


    export std::pmr::vector<VkImage> get_swapchain_images(
        VkDevice device,
        Swapchain swapchain,
        std::pmr::polymorphic_allocator<VkSurfaceFormatKHR> allocator = {}
    ) noexcept;


    export struct Swapchain_image_index
    {
        std::uint32_t value = 0;
    };

    export std::optional<Swapchain_image_index> acquire_next_image(
        VkDevice device,
        Swapchain swapchain,
        std::uint64_t timeout,
        std::optional<VkSemaphore> semaphore,
        std::optional<VkFence> fence
    ) noexcept;


    export VkResult queue_present(
        VkQueue queue,
        std::span<VkSemaphore const> semaphores_to_wait,
        Swapchain swapchain,
        Swapchain_image_index swapchain_image_index
    ) noexcept;
}