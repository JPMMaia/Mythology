module;

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>

#include <array>
#include <cassert>
#include <memory>
#include <memory_resource>
#include <span>
#include <string>
#include <variant>
#include <vector>

module mythology.sdl.startup_state;

import maia.renderer.vulkan;

import mythology.sdl.configuration;
import mythology.sdl.render_resources;
import mythology.sdl.sdl;
import mythology.sdl.state;
import mythology.sdl.vulkan;

namespace Mythology::SDL
{
    Startup_state::~Startup_state() noexcept
    {
    }

    std::unique_ptr<State> Startup_state::run()
    {
        std::size_t const number_of_frames_in_flight = 3;

        std::array<Window_configuration, 1> const window_configurations
        {
            Window_configuration
            {
                .title = "Main window",
                //.mode = Fullscreen_mode{.display_index = 0},
                .mode = Windowed_mode
                {
                    .offset = {0, 0},
                    .extent = {800, 600},
                },
            }
        };

        std::array<Surface_configuration, 1> const surface_configurations
        {
            Surface_configuration
            {
                .window_index = 0,
            },
        };

        std::array<Physical_device_configuration, 1> const physical_device_configurations
        {
            Physical_device_configuration
            {
                .vendor_ID = 4318,
                .device_ID = 7953,
            }
        };

        std::array<Device_configuration, 1> const device_configurations
        {
            Device_configuration
            {
                .physical_device_index = 0,
                .queues =
                {
                    Queue_create_info_configuration
                    {
                        .queue_family_index = 0,
                        .priorities = {0.5f},
                    }
                },
                .enabled_extensions =
                {
                    "VK_KHR_swapchain",
                },
            }
        };

        std::array<Queue_configuration, 1> const queue_configurations
        {
            Queue_configuration
            {
                .device_index = 0,
                .queue_family_index = 0,
                .queue_index = 0
            }
        };

        std::array<Swapchain_configuration, 1> const swapchain_configurations
        {
            Swapchain_configuration
            {
                .image_format = VK_FORMAT_B8G8R8A8_SRGB,
            }
        };

        SDL_instance sdl{SDL_INIT_VIDEO};

        std::pmr::vector<SDL_window> const windows = create_windows(sdl, window_configurations);

        std::pmr::vector<char const*> const required_instance_extensions =
            Mythology::SDL::Vulkan::get_sdl_required_instance_extensions({});

        Mythology::Render::Instance_resources instance_resources
        {
            Maia::Renderer::Vulkan::make_api_version(1, 2, 0),
            required_instance_extensions
        };

        std::pmr::vector<SDL_Window*> const surface_windows =
            select_surface_windows(surface_configurations, windows, {});

        Mythology::SDL::Vulkan::Surface_resources surface_resources
        {
            instance_resources.instance,
            surface_windows,
            {}
        };

        std::pmr::vector<VkPhysicalDevice> const physical_devices = get_physical_devices(
            physical_device_configurations,
            instance_resources.instance,
            {},
            {}
        );

        Mythology::SDL::Device_resources device_resources
        {
            device_configurations,
            physical_devices,
            {},
            {}
        };

        std::pmr::vector<VkQueue> const queues = get_queues(
            queue_configurations,
            device_resources.devices,
            {}
        );

        std::pmr::vector<VkDevice> const swapchain_devices = get_swapchain_devices(
            swapchain_configurations,
            device_resources.devices,
            {}
        );

        std::pmr::vector<SDL_Window*> const window_raw_pointers = get_window_raw_pointers(
            windows,
            {}
        );

        std::pmr::vector<VkExtent2D> const surface_image_extents = get_image_extents(
            surface_configurations,
            window_raw_pointers,
            {}
        );

        if (!validate_swapchain_queues(device_configurations, queue_configurations, swapchain_configurations, physical_devices, surface_resources.surfaces))
        {
            throw std::runtime_error{"Swapchain present queues are not compatible!"};
        }

        std::pmr::vector<VkSurfaceCapabilitiesKHR> const swapchain_surface_capabilities = get_swapchain_surface_capabilities(
            swapchain_configurations,
            device_configurations,
            physical_devices,
            surface_resources.surfaces,
            {}
        );

        Mythology::SDL::Swapchain_resources const swapchain_resources
        {
            swapchain_configurations,
            swapchain_devices,
            surface_resources.surfaces,
            surface_image_extents,
            swapchain_surface_capabilities,
            {}
        };

        Mythology::Render::Synchronization_resources const synchronization_resources
        {
            number_of_frames_in_flight,
            swapchain_devices,
            {}
        };

        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(5s);
        }

        // Create surfaces
        // Choose which physical devices and queue family indices to use
        // Create devices
        // Get queues
        // Create swapchains
        // Create swapchains image views
        // Create semaphores
        // Create fences

        // Load loading assets
        // Render black screen while loading assets

        

        // Return next state
        return {};
    }
}
