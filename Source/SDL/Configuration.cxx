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

    export struct Queue_configuration
    {
        std::uint32_t queue_family_index = 0;
        std::pmr::vector<float> priorities;

        std::uint32_t count() const noexcept;
    };

    export struct Device_configuration
    {
        std::uint32_t physical_device_index = 0;
        std::pmr::vector<Queue_configuration> queues;
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
}
