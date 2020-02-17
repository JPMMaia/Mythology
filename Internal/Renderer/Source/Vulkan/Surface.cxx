export module maia.renderer.vulkan.surface;

import maia.renderer.vulkan.device;
import maia.renderer.vulkan.physical_device;

import <vulkan/vulkan.h>;

import <cstdint>;
import <memory_resource>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    export struct Surface
    {
        VkSurfaceKHR value;
    };

    export bool is_surface_supported(
        Physical_device physical_device,
        Queue_family_index queue_family_index,
        Surface surface
    ) noexcept;

    export VkSurfaceCapabilitiesKHR get_surface_capabilities(
        Physical_device physical_device,
        Surface surface
    ) noexcept;

    export std::pmr::vector<VkSurfaceFormatKHR> get_surface_formats(
        Physical_device physical_device,
        Surface surface,
        std::pmr::polymorphic_allocator<VkSurfaceFormatKHR> allocator = {}
    ) noexcept;

    export std::pmr::vector<VkPresentModeKHR> get_surface_present_modes(
        Physical_device physical_device,
        Surface surface,
        std::pmr::polymorphic_allocator<VkPresentModeKHR> allocator = {}
    ) noexcept;
}