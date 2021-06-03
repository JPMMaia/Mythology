module;

#include <SDL2/SDL.h>
#include <vulkan/vulkan.hpp>

#include <cstdint>
#include <memory_resource>
#include <span>
#include <string>
#include <variant>
#include <vector>

export module mythology.sdl.configuration;

import mythology.sdl.render_resources;
import mythology.sdl.sdl;

namespace Mythology::SDL
{
    export struct Window_offset
    {
        int x = 0;
        int y = 0;
    };

    export struct Window_extent
    {
        int width = 0;
        int height = 0;
    };

    export struct Fullscreen_mode
    {
        unsigned int display_index;
    };

    export struct Windowed_mode
    {
        Window_offset offset;
        Window_extent extent;
    };

    export struct Window_configuration
    {
        std::pmr::string title;
        std::variant<Fullscreen_mode, Windowed_mode> mode;
    };

    export std::pmr::vector<Mythology::SDL::SDL_window> create_windows(
        Mythology::SDL::SDL_instance const& sdl,
        std::span<Window_configuration const> const window_configurations
    );

    export struct Surface_configuration
    {
        std::uint8_t window_index = 0;
    };

    export std::pmr::vector<SDL_Window*> select_surface_windows(
        std::span<Surface_configuration const> const surface_configurations,
        std::span<Mythology::SDL::SDL_window const> windows,
        std::pmr::polymorphic_allocator<> const& allocator
    );

    export struct Physical_device_configuration
    {
        std::uint32_t vendor_ID = 0;
        std::uint32_t device_ID = 0;
    };

    export std::pmr::vector<VkPhysicalDevice> get_physical_devices(
        std::span<Physical_device_configuration const> const configurations,
        VkInstance const instance,
        std::pmr::polymorphic_allocator<> const& allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    );

    export struct Queue_create_info_configuration
    {
        std::uint32_t queue_family_index = 0;
        std::pmr::vector<float> priorities;

        std::uint32_t count() const noexcept;
    };

    export struct Device_configuration
    {
        std::uint32_t physical_device_index = 0;
        std::pmr::vector<Queue_create_info_configuration> queues;
        std::pmr::vector<char const*> enabled_extensions;
    };

    export struct Device_resources
    {
        Device_resources(
            std::span<Device_configuration const> configurations,
            std::span<VkPhysicalDevice const> physical_devices,
            std::pmr::polymorphic_allocator<> const& allocator,
            std::pmr::polymorphic_allocator<> const& temporaries_allocator
        );
        Device_resources(Device_resources const&) = delete;
        Device_resources(Device_resources&& other) noexcept;
        ~Device_resources() noexcept;

        Device_resources& operator=(Device_resources const&) = delete;
        Device_resources& operator=(Device_resources&& other) noexcept;

        std::pmr::vector<VkDevice> devices;
    };

    export struct Queue_configuration
    {
        std::uint32_t device_index = 0;
        std::uint32_t queue_family_index = 0;
        std::uint32_t queue_index = 0;
    };

    export std::pmr::vector<VkQueue> get_queues(
        std::span<Queue_configuration const> const configurations,
        std::span<VkDevice const> const devices,
        std::pmr::polymorphic_allocator<> const& allocator
    );

    export struct Swapchain_configuration
    {
        std::uint32_t device_index = 0;
        std::uint32_t surface_index = 0;
        VkSwapchainCreateFlagsKHR flags = {};
        std::uint32_t minimum_image_count = 3;
        VkFormat image_format = VK_FORMAT_UNDEFINED;
        VkColorSpaceKHR image_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        std::uint32_t image_array_layers = 1;
        VkImageUsageFlags image_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        VkSharingMode image_sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
        std::pmr::vector<std::uint32_t> queue_family_indices = {};
        VkSurfaceTransformFlagBitsKHR pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
        bool clipped = true;

        std::uint32_t queue_family_index_count() const noexcept;
    };

    export std::pmr::vector<VkDevice> get_swapchain_devices(
        std::span<Swapchain_configuration const> configurations,
        std::span<VkDevice const> devices,
        std::pmr::polymorphic_allocator<> const& allocator
    );

    export std::pmr::vector<VkExtent2D> get_image_extents(
        std::span<Surface_configuration const> configurations,
        std::span<SDL_Window* const> windows,
        std::pmr::polymorphic_allocator<> const& allocator
    );

    export struct Swapchain_resources
    {
        Swapchain_resources(
            std::span<Swapchain_configuration const> configurations,
            std::span<VkDevice const> devices,
            std::span<VkSurfaceKHR const> surfaces,
            std::span<VkExtent2D const> image_extents,
            std::pmr::polymorphic_allocator<> const& allocator
        );
        Swapchain_resources(Swapchain_resources const&) = delete;
        Swapchain_resources(Swapchain_resources&& other) noexcept;
        ~Swapchain_resources() noexcept;

        Swapchain_resources& operator=(Swapchain_resources const&) = delete;
        Swapchain_resources& operator=(Swapchain_resources&& other) noexcept;

        std::pmr::vector<VkDevice> devices;
        std::pmr::vector<VkSwapchainKHR> swapchains;
    };
}
