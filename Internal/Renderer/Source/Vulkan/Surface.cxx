export module maia.renderer.vulkan.surface;


import <vulkan/vulkan.h>;

import <cstdint>;
import <memory_resource>;
import <optional>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    export struct Surface
    {
        VkSurfaceKHR value = VK_NULL_HANDLE;
    };

    export bool is_surface_supported(
        VkPhysicalDevice physical_device,
        Queue_family_index queue_family_index,
        Surface surface
    ) noexcept;

    export VkSurfaceCapabilitiesKHR get_surface_capabilities(
        VkPhysicalDevice physical_device,
        Surface surface
    ) noexcept;

    export std::pmr::vector<VkSurfaceFormatKHR> get_surface_formats(
        VkPhysicalDevice physical_device,
        Surface surface,
        std::pmr::polymorphic_allocator<VkSurfaceFormatKHR> allocator = {}
    ) noexcept;

    export std::pmr::vector<VkPresentModeKHR> get_surface_present_modes(
        VkPhysicalDevice physical_device,
        Surface surface,
        std::pmr::polymorphic_allocator<VkPresentModeKHR> allocator = {}
    ) noexcept;

    export void destroy_surface(
        VkInstance instance,
        Surface surface,
        VkAllocationCallbacks const* allocator = {}) noexcept;
}