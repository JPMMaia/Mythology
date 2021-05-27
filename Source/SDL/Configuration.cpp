module;

#include <SDL2/SDL.h>
#include <vulkan/vulkan.hpp>

#include <algorithm>
#include <cstdint>
#include <memory_resource>
#include <span>
#include <string>
#include <variant>
#include <vector>

module mythology.sdl.configuration;

import maia.renderer.vulkan;

import mythology.sdl.render_resources;
import mythology.sdl.sdl;
import mythology.sdl.vulkan;

namespace Mythology::SDL
{
    std::pmr::vector<Mythology::SDL::SDL_window> create_windows(
        SDL_instance const& sdl,
        std::span<Window_configuration const> const window_configurations
    )
    {
        std::pmr::vector<Mythology::SDL::SDL_window> windows;
        windows.reserve(window_configurations.size());

        constexpr Uint32 common_flags = SDL_WINDOW_VULKAN;

        int const number_of_displays = SDL_GetNumVideoDisplays();

        for (Window_configuration const& configuration : window_configurations)
        {
            if (configuration.mode.index() == 0)
            {
                Fullscreen_mode const& fullscreen_mode = std::get<Fullscreen_mode>(configuration.mode);

                if (fullscreen_mode.display_index < number_of_displays)
                {
                    SDL_Rect bounds = {};
                    SDL_GetDisplayBounds(fullscreen_mode.display_index, &bounds);

                    windows.push_back(
                        SDL_window
                        {
                            sdl,
                            configuration.title.c_str(),
                            bounds.x,
                            bounds.y,
                            bounds.w,
                            bounds.h,
                            SDL_WINDOW_FULLSCREEN_DESKTOP | common_flags
                        }
                    );
                }
                else
                {
                    throw std::runtime_error{"Display not found!"};
                }
            }
            else
            {
                assert(configuration.mode.index() == 1);

                Windowed_mode const& windowed_mode = std::get<Windowed_mode>(configuration.mode);

                windows.push_back(
                    SDL_window
                    {
                        sdl,
                        configuration.title.c_str(),
                        windowed_mode.offset.x,
                        windowed_mode.offset.y,
                        windowed_mode.extent.width,
                        windowed_mode.extent.height,
                        common_flags
                    }
                );
            }
        }

        return windows;
    }

    std::pmr::vector<SDL_Window*> select_surface_windows(
        std::span<Surface_configuration const> const surface_configurations,
        std::span<SDL_window const> windows,
        std::pmr::polymorphic_allocator<> const& allocator
    )
    {
        std::pmr::vector<SDL_Window*> surface_windows{allocator};
        surface_windows.reserve(surface_configurations.size());

        for (Surface_configuration const& configuration : surface_configurations)
        {
            SDL_Window* const window =
                configuration.window_index < windows.size() ?
                windows[configuration.window_index].get() :
                nullptr;

            surface_windows.push_back(window);
        }

        return surface_windows;
    }

    std::pmr::vector<VkPhysicalDevice> get_physical_devices(
        std::span<Physical_device_configuration const> const configurations,
        VkInstance const instance,
        std::pmr::polymorphic_allocator<> const& allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    )
    {
        using Maia::Renderer::Vulkan::enumerate_physical_devices;
        using Maia::Renderer::Vulkan::get_physical_device_properties;

        std::pmr::vector<VkPhysicalDevice> physical_devices{allocator};
        physical_devices.reserve(configurations.size());

        std::pmr::vector<VkPhysicalDevice> const actual_physical_devices =
            enumerate_physical_devices(instance, temporaries_allocator);

        std::pmr::vector<VkPhysicalDeviceProperties> const properties = [&]
        {
            std::pmr::vector<VkPhysicalDeviceProperties> properties{temporaries_allocator};
            properties.reserve(actual_physical_devices.size());
            
            for (VkPhysicalDevice const physical_device : actual_physical_devices)
            {
                properties.push_back(get_physical_device_properties(physical_device));
            }

            return properties;
        }();

        for (Physical_device_configuration const configuration : configurations)
        {
            auto const is_same_physical_device = [configuration] (VkPhysicalDeviceProperties const& properties)
            {
                return configuration.vendor_ID == properties.vendorID && configuration.device_ID == properties.deviceID;
            };

            auto const location = std::find_if(properties.begin(), properties.end(), is_same_physical_device);

            VkPhysicalDevice const physical_device = 
                location != properties.end() ?
                actual_physical_devices[std::distance(properties.begin(), location)] :
                VkPhysicalDevice{VK_NULL_HANDLE};

            physical_devices.push_back(physical_device);
        }

        for (VkPhysicalDevice const physical_device : actual_physical_devices)
        {
            auto const location = std::find(physical_devices.begin(), physical_devices.end(), physical_device);

            if (location == physical_devices.end())
            {
                physical_devices.push_back(physical_device);
            }
        }

        return physical_devices;
    }

    std::uint32_t Queue_create_info_configuration::count() const noexcept
    {
        return static_cast<std::uint32_t>(priorities.size());
    }

    namespace
    {
        std::pmr::vector<VkDeviceQueueCreateInfo> create_device_queue_create_infos(
            std::span<Queue_create_info_configuration const> const configurations,
            std::pmr::polymorphic_allocator<> const& allocator
        )
        {
            std::pmr::vector<VkDeviceQueueCreateInfo> queue_create_infos{allocator};
            queue_create_infos.reserve(configurations.size());

            for (Queue_create_info_configuration const configuration : configurations)
            {
                VkDeviceQueueCreateInfo const create_info
                {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .queueFamilyIndex = configuration.queue_family_index,
                    .queueCount = configuration.count(),
                    .pQueuePriorities = configuration.priorities.data(),
                };

                queue_create_infos.push_back(create_info);
            }

            return queue_create_infos;
        }
    }

    std::pmr::vector<VkDevice> create_devices(
        std::span<Device_configuration const> const configurations,
        std::span<VkPhysicalDevice const> const physical_devices,
        std::pmr::polymorphic_allocator<> const& allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    )
    {
        using Maia::Renderer::Vulkan::create_device_queue_create_info;
        using Maia::Renderer::Vulkan::create_device;

        std::pmr::vector<VkDevice> devices{allocator};
        devices.reserve(configurations.size());

        for (Device_configuration const configuration : configurations)
        {
            VkPhysicalDevice const physical_device = physical_devices[configuration.physical_device_index];

            std::pmr::vector<VkDeviceQueueCreateInfo> const queue_create_infos =
                create_device_queue_create_infos(configuration.queues, temporaries_allocator);

            VkDevice const device = create_device(
                physical_device,
                queue_create_infos,
                configuration.enabled_extensions
            );
            
            devices.push_back(device);
        }

        return devices;
    }

    Device_resources::Device_resources(
        std::span<Device_configuration const> configurations,
        std::span<VkPhysicalDevice const> physical_devices,
        std::pmr::polymorphic_allocator<> const& allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    ) :
        devices{create_devices(configurations, physical_devices, allocator, temporaries_allocator)}
    {
    }
    
    Device_resources::Device_resources(Device_resources&& other) noexcept :
        devices{std::move(other.devices)}
    {
    }
    
    Device_resources::~Device_resources() noexcept
    {
        for (VkDevice const device : this->devices)
        {
            vkDestroyDevice(device, nullptr);
        }
    }

    Device_resources& Device_resources::operator=(Device_resources&& other) noexcept
    {
        std::swap(this->devices, other.devices);

        return *this;
    }

    std::pmr::vector<VkQueue> get_queues(
        std::span<Queue_configuration const> const configurations,
        std::span<VkDevice const> const devices,
        std::pmr::polymorphic_allocator<> const& allocator
    )
    {
        std::pmr::vector<VkQueue> queues{allocator};
        queues.reserve(configurations.size());

        for (Queue_configuration const& configuration : configurations)
        {
            VkDevice const device = devices[configuration.device_index];

            VkQueue queue = VK_NULL_HANDLE;
            vkGetDeviceQueue(
                device,
                configuration.queue_family_index,
                configuration.queue_index,
                &queue
            );

            queues.push_back(queue);
        }

        return queues;
    }
}
