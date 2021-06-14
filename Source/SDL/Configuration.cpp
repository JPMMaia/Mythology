module;

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>

#include <algorithm>
#include <cstdint>
#include <memory_resource>
#include <span>
#include <string>
#include <variant>
#include <vector>

module mythology.sdl.configuration;

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

    std::pmr::vector<vk::PhysicalDevice> get_physical_devices(
        std::span<Physical_device_configuration const> const configurations,
        vk::Instance const instance,
        std::pmr::polymorphic_allocator<> const& allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    )
    {
        std::pmr::vector<vk::PhysicalDevice> physical_devices{allocator};
        physical_devices.reserve(configurations.size());

        std::pmr::polymorphic_allocator<vk::PhysicalDevice> physical_device_allocator{temporaries_allocator};
        std::pmr::vector<vk::PhysicalDevice> const actual_physical_devices =
            instance.enumeratePhysicalDevices(physical_device_allocator);

        std::pmr::vector<vk::PhysicalDeviceProperties> const properties = [&]
        {
            std::pmr::vector<vk::PhysicalDeviceProperties> properties{temporaries_allocator};
            properties.reserve(actual_physical_devices.size());
            
            for (vk::PhysicalDevice const physical_device : actual_physical_devices)
            {
                vk::PhysicalDeviceProperties const physical_device_properties =
                    physical_device.getProperties();

                properties.push_back(physical_device_properties);
            }

            return properties;
        }();

        for (Physical_device_configuration const configuration : configurations)
        {
            auto const is_same_physical_device = [configuration] (vk::PhysicalDeviceProperties const& properties)
            {
                return configuration.vendor_ID == properties.vendorID && configuration.device_ID == properties.deviceID;
            };

            auto const location = std::find_if(properties.begin(), properties.end(), is_same_physical_device);

            vk::PhysicalDevice const physical_device = 
                location != properties.end() ?
                actual_physical_devices[std::distance(properties.begin(), location)] :
                vk::PhysicalDevice{};

            physical_devices.push_back(physical_device);
        }

        for (vk::PhysicalDevice const physical_device : actual_physical_devices)
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
        std::pmr::vector<vk::DeviceQueueCreateInfo> create_device_queue_create_infos(
            std::span<Queue_create_info_configuration const> const configurations,
            std::pmr::polymorphic_allocator<> const& allocator
        )
        {
            std::pmr::vector<vk::DeviceQueueCreateInfo> queue_create_infos{allocator};
            queue_create_infos.reserve(configurations.size());

            for (Queue_create_info_configuration const& configuration : configurations)
            {
                vk::DeviceQueueCreateInfo const create_info
                {
                    .flags = {},
                    .queueFamilyIndex = configuration.queue_family_index,
                    .queueCount = configuration.count(),
                    .pQueuePriorities = configuration.priorities.data(),
                };

                queue_create_infos.push_back(create_info);
            }

            return queue_create_infos;
        }
    }

    std::pmr::vector<vk::Device> create_devices(
        std::span<Device_configuration const> const configurations,
        std::span<vk::PhysicalDevice const> const physical_devices,
        std::pmr::polymorphic_allocator<> const& allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    )
    {
        std::pmr::vector<vk::Device> devices{allocator};
        devices.reserve(configurations.size());

        for (Device_configuration const configuration : configurations)
        {
            vk::PhysicalDevice const physical_device = physical_devices[configuration.physical_device_index];

            std::pmr::vector<vk::DeviceQueueCreateInfo> const queue_create_infos =
                create_device_queue_create_infos(configuration.queues, temporaries_allocator);

            vk::DeviceCreateInfo const create_info
            {
                .flags = {},
                .queueCreateInfoCount = static_cast<std::uint32_t>(queue_create_infos.size()),
                .pQueueCreateInfos = queue_create_infos.data(),
                .enabledLayerCount = {},
                .ppEnabledLayerNames = {},
                .enabledExtensionCount = static_cast<std::uint32_t>(configuration.enabled_extensions.size()),
                .ppEnabledExtensionNames = configuration.enabled_extensions.data(),
                .pEnabledFeatures = {},
            };

            vk::Device const device = physical_device.createDevice(
                create_info
            );
            
            devices.push_back(device);
        }

        return devices;
    }

    Device_resources::Device_resources(
        std::span<Device_configuration const> configurations,
        std::span<vk::PhysicalDevice const> physical_devices,
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
        for (vk::Device const device : this->devices)
        {
            device.destroy();
        }
    }

    Device_resources& Device_resources::operator=(Device_resources&& other) noexcept
    {
        std::swap(this->devices, other.devices);

        return *this;
    }

    std::pmr::vector<vk::Queue> get_queues(
        std::span<Queue_configuration const> const configurations,
        std::span<vk::Device const> const devices,
        std::pmr::polymorphic_allocator<> const& allocator
    )
    {
        std::pmr::vector<vk::Queue> queues{allocator};
        queues.reserve(configurations.size());

        for (Queue_configuration const& configuration : configurations)
        {
            vk::Device const device = devices[configuration.device_index];

            vk::Queue const queue = device.getQueue(
                configuration.queue_family_index,
                configuration.queue_index
            );

            queues.push_back(queue);
        }

        return queues;
    }

    std::uint32_t Swapchain_configuration::queue_family_index_count() const noexcept
    {
        return static_cast<std::uint32_t>(queue_family_indices.size());
    }

    std::pmr::vector<vk::Device> get_swapchain_devices(
        std::span<Swapchain_configuration const> configurations,
        std::span<vk::Device const> devices,
        std::pmr::polymorphic_allocator<> const& allocator
    )
    {
        std::pmr::vector<vk::Device> swapchain_devices{allocator};
        swapchain_devices.reserve(configurations.size());

        for (Swapchain_configuration const configuration : configurations)
        {
            swapchain_devices.push_back(
                devices[configuration.device_index]
            );
        }

        return swapchain_devices;
    }

    std::pmr::vector<vk::Extent2D> get_image_extents(
        std::span<Surface_configuration const> configurations,
        std::span<SDL_Window* const> windows,
        std::pmr::polymorphic_allocator<> const& allocator
    )
    {
        auto const get_drawable_size = [windows] (Surface_configuration const& configuration) -> vk::Extent2D
        {
            SDL_Window* const window = windows[configuration.window_index];

            int width = 0;
            int height = 0;
            SDL_Vulkan_GetDrawableSize(window, &width, &height);
            assert(width >= 0 && height >= 0);

            return
            {
                .width = static_cast<std::uint32_t>(width),
                .height = static_cast<std::uint32_t>(height),
            };
        };

        std::pmr::vector<vk::Extent2D> image_extents{allocator};
        image_extents.resize(configurations.size());

        std::transform(
            configurations.begin(),
            configurations.end(),
            image_extents.begin(),
            get_drawable_size
        );

        return image_extents;
    }

    bool validate_swapchain_queues(
        std::span<Device_configuration const> const device_configurations,
        std::span<Queue_configuration const> const queue_configurations,
        std::span<Swapchain_configuration const> const swapchain_configurations,
        std::span<vk::PhysicalDevice const> const physical_devices,
        std::span<vk::SurfaceKHR const> const surfaces
    ) noexcept
    {
        auto const queue_supports_physical_device_surface = [=] (Swapchain_configuration const& swapchain_configuration) -> bool
        {
            Queue_configuration const& queue_configuration = queue_configurations[swapchain_configuration.queue_to_present_index];
            Device_configuration const& device_configuration = device_configurations[queue_configuration.device_index];
            vk::PhysicalDevice const physical_device = physical_devices[device_configuration.physical_device_index];
            vk::SurfaceKHR const surface = surfaces[swapchain_configuration.surface_index];

            vk::Bool32 const is_supported = physical_device.getSurfaceSupportKHR(
                queue_configuration.queue_family_index,
                surface
            );

            return is_supported == VK_TRUE;
        };

        return std::all_of(
            swapchain_configurations.begin(),
            swapchain_configurations.end(),
            queue_supports_physical_device_surface
        );
    }

    std::pmr::vector<vk::SurfaceCapabilitiesKHR> get_swapchain_surface_capabilities(
        std::span<Swapchain_configuration const> const swapchain_configurations,
        std::span<Device_configuration const> const device_configurations,
        std::span<vk::PhysicalDevice const> const physical_devices,
        std::span<vk::SurfaceKHR const> const surfaces,
        std::pmr::polymorphic_allocator<> const& allocator
    )
    {
        auto const get_surface_capabilities = [=] (Swapchain_configuration const& swapchain_configuration) -> vk::SurfaceCapabilitiesKHR
        {
            Device_configuration const& device_configuration = device_configurations[swapchain_configuration.device_index];
            vk::PhysicalDevice const physical_device = physical_devices[device_configuration.physical_device_index];
            vk::SurfaceKHR const surface = surfaces[swapchain_configuration.surface_index];

            vk::SurfaceCapabilitiesKHR const capabilities = physical_device.getSurfaceCapabilitiesKHR(
                surface
            );

            return capabilities;
        };

        std::pmr::vector<vk::SurfaceCapabilitiesKHR> capabilities{allocator};
        capabilities.resize(swapchain_configurations.size());

        std::transform(
            swapchain_configurations.begin(),
            swapchain_configurations.end(),
            capabilities.begin(),
            get_surface_capabilities
        );

        return capabilities;
    }

    namespace
    {
        vk::SwapchainKHR create_swapchain(
            Swapchain_configuration const configuration,
            vk::Device const device,
            vk::SurfaceKHR const surface,
            vk::Extent2D const image_extent,
            vk::SurfaceTransformFlagBitsKHR const pre_transfrm
        )
        {
            vk::SwapchainCreateInfoKHR const create_info
            {
                .flags = configuration.flags,
                .surface = surface,
                .minImageCount = configuration.minimum_image_count,
                .imageFormat = configuration.image_format,
                .imageColorSpace = configuration.image_color_space,
                .imageExtent = image_extent,
                .imageArrayLayers = configuration.image_array_layers,
                .imageUsage = configuration.image_usage,
                .imageSharingMode = configuration.image_sharing_mode,
                .queueFamilyIndexCount = configuration.queue_family_index_count(),
                .pQueueFamilyIndices = !configuration.queue_family_indices.empty() ? configuration.queue_family_indices.data() : nullptr,
                .preTransform = pre_transfrm,
                .compositeAlpha = configuration.composite_alpha,
                .presentMode = configuration.present_mode,
                .clipped = configuration.clipped,
                .oldSwapchain = {},
            };

            vk::SwapchainKHR const swapchain = device.createSwapchainKHR(
                create_info,
                nullptr
            );

            return swapchain;
        }

        std::pmr::vector<vk::SwapchainKHR> create_swapchains(
            std::span<Swapchain_configuration const> const configurations,
            std::span<vk::Device const> const devices,
            std::span<vk::SurfaceKHR const> const surfaces,
            std::span<vk::Extent2D const> const image_extents,
            std::span<vk::SurfaceCapabilitiesKHR const> const swapchain_surface_capabilities,
            std::pmr::polymorphic_allocator<> const& allocator
        )
        {
            assert(surfaces.size() == image_extents.size());

            std::pmr::vector<vk::SwapchainKHR> swapchains{allocator};
            swapchains.reserve(configurations.size());

            for (std::size_t swapchain_index = 0; swapchain_index < configurations.size(); ++swapchain_index)
            {
                Swapchain_configuration const& configuration = configurations[swapchain_index];
                vk::Device const device = devices[configuration.device_index];
                vk::SurfaceKHR const surface = surfaces[configuration.surface_index];
                vk::Extent2D const image_extent = image_extents[configuration.surface_index];
                vk::SurfaceCapabilitiesKHR const& surface_capabilities = swapchain_surface_capabilities[swapchain_index];

                vk::SwapchainKHR const swapchain = create_swapchain(
                    configuration,
                    device,
                    surface,
                    image_extent,
                    surface_capabilities.currentTransform
                );

                swapchains.push_back(swapchain);
            }

            return swapchains;
        }

        std::pmr::vector<std::pmr::vector<vk::Image>> get_swapchain_images(
            std::span<Swapchain_configuration const> const configurations,
            std::span<vk::Device const> const devices,
            std::span<vk::SwapchainKHR const> const swapchains,
            std::pmr::polymorphic_allocator<> const& allocator
        )
        {
            assert(configurations.size() == swapchains.size());

            std::pmr::vector<std::pmr::vector<vk::Image>> swapchain_images{allocator};
            swapchain_images.reserve(configurations.size());

            for (std::size_t index = 0; index < configurations.size(); ++index)
            {
                Swapchain_configuration const& configuration = configurations[index];
                vk::Device const device = devices[configuration.device_index];
                vk::SwapchainKHR const swapchain = swapchains[index];

                std::pmr::polymorphic_allocator<vk::Image> image_allocator{allocator};
                std::pmr::vector<vk::Image> images = 
                    device.getSwapchainImagesKHR(
                        swapchain,
                        image_allocator
                    );
                
                swapchain_images.push_back(
                    std::move(images)
                );
            }

            return swapchain_images;
        }
    }

    Swapchain_resources::Swapchain_resources(
        std::span<Swapchain_configuration const> const configurations,
        std::span<vk::Device const> const devices,
        std::span<vk::SurfaceKHR const> const surfaces,
        std::span<vk::Extent2D const> const image_extents,
        std::span<vk::SurfaceCapabilitiesKHR const> const swapchain_surface_capabilities,
        std::pmr::polymorphic_allocator<> const& allocator
    ) :
        devices{devices.begin(), devices.end(), allocator},
        swapchains{create_swapchains(configurations, devices, surfaces, image_extents, swapchain_surface_capabilities, allocator)},
        images{get_swapchain_images(configurations, devices, swapchains, allocator)}
    {
        if (devices.size() != swapchains.size())
        {
            throw std::runtime_error{"For each swapchain, there must be one corresponding device!"};
        }
    }

    Swapchain_resources::Swapchain_resources(Swapchain_resources&& other) noexcept :
        devices{std::move(other.devices)},
        swapchains{std::move(other.swapchains)},
        images(std::move(other.images))
    {
    }

    Swapchain_resources::~Swapchain_resources() noexcept
    {
        for (std::size_t swapchain_index = 0; swapchain_index < this->swapchains.size(); ++swapchain_index)
        {
            vk::Device const device = this->devices[swapchain_index];
            vk::SwapchainKHR const swapchain = this->swapchains[swapchain_index];

            vkDestroySwapchainKHR(device, swapchain, nullptr);
        }
    }

    Swapchain_resources& Swapchain_resources::operator=(Swapchain_resources&& other) noexcept
    {
        std::swap(this->devices, other.devices);
        std::swap(this->swapchains, other.swapchains);
        std::swap(this->images, other.images);

        return *this;
    }


    std::pmr::vector<vk::Device> get_queue_devices(
        std::span<Queue_configuration const> const configurations,
        std::span<vk::Device const> const devices,
        std::pmr::polymorphic_allocator<> const& allocator
    )
    {
        auto const get_queue_device = [devices] (Queue_configuration const& configuration) -> vk::Device
        {
            return devices[configuration.device_index];
        };

        std::pmr::vector<vk::Device> queue_devices{allocator};
        queue_devices.resize(configurations.size());

        std::transform(
            configurations.begin(),
            configurations.end(),
            queue_devices.begin(),
            get_queue_device
        );

        return queue_devices;
    }

    std::pmr::vector<std::uint32_t> get_queue_family_indices(
        std::span<Queue_configuration const> const configurations,
        std::pmr::polymorphic_allocator<> const& allocator
    )
    {
        auto const get_queue_family_index = [] (Queue_configuration const& configuration) -> std::uint32_t
        {
            return configuration.queue_family_index;
        };

        std::pmr::vector<std::uint32_t> queue_family_indices{allocator};
        queue_family_indices.resize(configurations.size());

        std::transform(
            configurations.begin(),
            configurations.end(),
            queue_family_indices.begin(),
            get_queue_family_index
        );

        return queue_family_indices;
    }
}
