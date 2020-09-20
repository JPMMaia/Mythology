module maia.renderer.vulkan.surface;

import maia.renderer.vulkan.check;

import <vulkan/vulkan.h>;

import <cstdint>;
import <memory_resource>;
import <optional>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    bool is_surface_supported(
        VkPhysicalDevice const physical_device,
        Queue_family_index const queue_family_index,
        VkSurfaceKHR const surface
    ) noexcept
    {
        VkBool32 is_supported = VK_FALSE;
        check_result(
            vkGetPhysicalDeviceSurfaceSupportKHR(
                physical_device,
                queue_family_index.value,
                surface,
                &is_supported
            )
        );

        return is_supported == VK_TRUE;
    }

    VkSurfaceCapabilitiesKHR get_surface_capabilities(
        VkPhysicalDevice const physical_device,
        VkSurfaceKHR const surface
    ) noexcept
    {
        VkSurfaceCapabilitiesKHR surface_capabilities = {};
        check_result(
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                physical_device,
                surface,
                &surface_capabilities
            )
        );
        
        return surface_capabilities;
    }

    std::pmr::vector<VkSurfaceFormatKHR> get_surface_formats(
        VkPhysicalDevice const physical_device,
        VkSurfaceKHR const surface,
        std::pmr::polymorphic_allocator<VkSurfaceFormatKHR> const allocator
    ) noexcept
    {
        std::uint32_t surface_format_count = 0;
        check_result(
            vkGetPhysicalDeviceSurfaceFormatsKHR(
                physical_device,
                surface,
                &surface_format_count,
                nullptr
            )
        );

        if (surface_format_count > 0)
        {
            std::pmr::vector<VkSurfaceFormatKHR> surface_formats{surface_format_count, allocator};

            check_result(
                vkGetPhysicalDeviceSurfaceFormatsKHR(
                    physical_device,
                    surface,
                    &surface_format_count,
                    surface_formats.data()
                )
            );

            return surface_formats;
        }
        else
        {
            return {};
        }
    }

    std::pmr::vector<VkPresentModeKHR> get_surface_present_modes(
        VkPhysicalDevice const physical_device,
        VkSurfaceKHR const surface,
        std::pmr::polymorphic_allocator<VkPresentModeKHR> const allocator
    ) noexcept
    {
        std::uint32_t present_modes_count = 0;
        check_result(
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                physical_device,
                surface,
                &present_modes_count,
                nullptr
            )
        );

        if (present_modes_count > 0)
        {
            std::pmr::vector<VkPresentModeKHR> present_modes{present_modes_count, allocator};

            check_result(
                vkGetPhysicalDeviceSurfacePresentModesKHR(
                    physical_device,
                    surface,
                    &present_modes_count,
                    present_modes.data()
                )
            );

            return present_modes;
        }
        else
        {
            return {};
        }
    }

    void destroy_surface(VkInstance const instance, VkSurfaceKHR const surface, VkAllocationCallbacks const* const allocator) noexcept
    {
        vkDestroySurfaceKHR(instance, surface, allocator);
    }
}