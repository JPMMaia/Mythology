module;

#include <nlohmann/json.hpp>
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

import maia.renderer.vulkan.serializer;

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
                    throw std::runtime_error{ "Display not found!" };
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
        std::pmr::vector<SDL_Window*> surface_windows{ allocator };
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
        std::pmr::vector<vk::PhysicalDevice> physical_devices{ allocator };
        physical_devices.reserve(configurations.size());

        std::pmr::polymorphic_allocator<vk::PhysicalDevice> physical_device_allocator{ temporaries_allocator };
        std::pmr::vector<vk::PhysicalDevice> const actual_physical_devices =
            instance.enumeratePhysicalDevices(physical_device_allocator);

        std::pmr::vector<vk::PhysicalDeviceProperties> const properties = [&]
        {
            std::pmr::vector<vk::PhysicalDeviceProperties> properties{ temporaries_allocator };
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
            auto const is_same_physical_device = [configuration](vk::PhysicalDeviceProperties const& properties)
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
            std::pmr::vector<vk::DeviceQueueCreateInfo> queue_create_infos{ allocator };
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
        std::pmr::vector<vk::Device> devices{ allocator };
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
        devices{ create_devices(configurations, physical_devices, allocator, temporaries_allocator) }
    {
    }

    Device_resources::Device_resources(Device_resources&& other) noexcept :
        devices{ std::move(other.devices) }
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
        std::pmr::vector<vk::Queue> queues{ allocator };
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
        std::pmr::vector<vk::Device> swapchain_devices{ allocator };
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
        auto const get_drawable_size = [windows](Surface_configuration const& configuration) -> vk::Extent2D
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

        std::pmr::vector<vk::Extent2D> image_extents{ allocator };
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
        auto const queue_supports_physical_device_surface = [=](Swapchain_configuration const& swapchain_configuration) -> bool
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
        auto const get_surface_capabilities = [=](Swapchain_configuration const& swapchain_configuration) -> vk::SurfaceCapabilitiesKHR
        {
            Device_configuration const& device_configuration = device_configurations[swapchain_configuration.device_index];
            vk::PhysicalDevice const physical_device = physical_devices[device_configuration.physical_device_index];
            vk::SurfaceKHR const surface = surfaces[swapchain_configuration.surface_index];

            vk::SurfaceCapabilitiesKHR const capabilities = physical_device.getSurfaceCapabilitiesKHR(
                surface
            );

            return capabilities;
        };

        std::pmr::vector<vk::SurfaceCapabilitiesKHR> capabilities{ allocator };
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

            std::pmr::vector<vk::SwapchainKHR> swapchains{ allocator };
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

            std::pmr::vector<std::pmr::vector<vk::Image>> swapchain_images{ allocator };
            swapchain_images.reserve(configurations.size());

            for (std::size_t index = 0; index < configurations.size(); ++index)
            {
                Swapchain_configuration const& configuration = configurations[index];
                vk::Device const device = devices[configuration.device_index];
                vk::SwapchainKHR const swapchain = swapchains[index];

                std::pmr::polymorphic_allocator<vk::Image> image_allocator{ allocator };
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
        devices{ devices.begin(), devices.end(), allocator },
        swapchains{ create_swapchains(configurations, devices, surfaces, image_extents, swapchain_surface_capabilities, allocator) },
        images{ get_swapchain_images(configurations, devices, swapchains, allocator) }
    {
        if (devices.size() != swapchains.size())
        {
            throw std::runtime_error{ "For each swapchain, there must be one corresponding device!" };
        }
    }

    Swapchain_resources::Swapchain_resources(Swapchain_resources&& other) noexcept :
        devices{ std::move(other.devices) },
        swapchains{ std::move(other.swapchains) },
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


    std::pmr::vector<vk::Format> get_swapchain_image_formats(
        std::span<Swapchain_configuration const> const configurations,
        std::pmr::polymorphic_allocator<> const& allocator
    )
    {
        auto const get_format = [](Swapchain_configuration const& configuration) -> vk::Format
        {
            return configuration.image_format;
        };

        std::pmr::vector<vk::Format> formats{ allocator };
        formats.resize(configurations.size());

        std::transform(
            configurations.begin(),
            configurations.end(),
            formats.begin(),
            get_format
        );

        return formats;
    }

    std::pmr::vector<vk::Device> get_queue_devices(
        std::span<Queue_configuration const> const configurations,
        std::span<vk::Device const> const devices,
        std::pmr::polymorphic_allocator<> const& allocator
    )
    {
        auto const get_queue_device = [devices](Queue_configuration const& configuration) -> vk::Device
        {
            return devices[configuration.device_index];
        };

        std::pmr::vector<vk::Device> queue_devices{ allocator };
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
        auto const get_queue_family_index = [](Queue_configuration const& configuration) -> std::uint32_t
        {
            return configuration.queue_family_index;
        };

        std::pmr::vector<std::uint32_t> queue_family_indices{ allocator };
        queue_family_indices.resize(configurations.size());

        std::transform(
            configurations.begin(),
            configurations.end(),
            queue_family_indices.begin(),
            get_queue_family_index
        );

        return queue_family_indices;
    }


    std::pmr::vector<vk::Image> get_input_images(
        std::span<Render_pipeline_input_configuration const> const inputs,
        std::span<std::pmr::vector<vk::Image> const> const swapchain_images,
        std::span<std::uint32_t const> const swapchain_image_indices,
        std::pmr::polymorphic_allocator<> const& allocator
    )
    {
        assert(swapchain_images.size() == swapchain_image_indices.size());

        auto const get_input_image = [=](Render_pipeline_input_configuration const& input) -> vk::Image
        {
            std::uint32_t const swapchain_image_index = swapchain_image_indices[input.swapchain_index];
            return swapchain_images[input.swapchain_index][swapchain_image_index];
        };

        std::pmr::vector<vk::Image> input_images{ allocator };
        input_images.resize(inputs.size());

        std::transform(
            inputs.begin(),
            inputs.end(),
            input_images.begin(),
            get_input_image
        );

        return input_images;
    }

    std::pmr::vector<vk::ImageView> get_input_image_views(
        std::span<Render_pipeline_input_configuration const> const inputs,
        std::span<std::pmr::vector<vk::ImageView> const> const swapchain_image_views,
        std::span<std::uint32_t const> const swapchain_image_indices,
        std::pmr::polymorphic_allocator<> const& allocator
    )
    {
        assert(swapchain_image_views.size() == swapchain_image_indices.size());

        auto const get_input_image_view = [=](Render_pipeline_input_configuration const& input) -> vk::ImageView
        {
            std::uint32_t const swapchain_image_index = swapchain_image_indices[input.swapchain_index];
            return swapchain_image_views[input.swapchain_index][swapchain_image_index];
        };

        std::pmr::vector<vk::ImageView> input_image_views{ allocator };
        input_image_views.resize(inputs.size());

        std::transform(
            inputs.begin(),
            inputs.end(),
            input_image_views.begin(),
            get_input_image_view
        );

        return input_image_views;
    }

    std::pmr::vector<vk::Framebuffer> get_input_framebuffers(
        std::span<Render_pipeline_input_configuration const> inputs,
        std::span<std::pmr::vector<vk::Framebuffer> const> swapchain_framebuffers,
        std::span<std::uint32_t const> swapchain_image_indices,
        std::pmr::polymorphic_allocator<> const& allocator
    )
    {
        assert(swapchain_framebuffers.size() == swapchain_image_indices.size());

        auto const get_input_framebuffer = [=](Render_pipeline_input_configuration const& input) -> vk::Framebuffer
        {
            std::uint32_t const swapchain_image_index = swapchain_image_indices[input.swapchain_index];
            return swapchain_framebuffers[input.swapchain_index][swapchain_image_index];
        };

        std::pmr::vector<vk::Framebuffer> input_framebuffers{ allocator };
        input_framebuffers.resize(inputs.size());

        std::transform(
            inputs.begin(),
            inputs.end(),
            input_framebuffers.begin(),
            get_input_framebuffer
        );

        return input_framebuffers;
    }

    std::pmr::vector<vk::ImageSubresourceRange> get_image_subresource_ranges(
        std::span<Render_pipeline_input_configuration const> const inputs,
        std::pmr::polymorphic_allocator<> const& allocator
    )
    {
        auto const generate_image_subresource_range = []() -> vk::ImageSubresourceRange
        {
            return
            {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            };
        };

        std::pmr::vector<vk::ImageSubresourceRange> image_subresource_ranges{ allocator };
        image_subresource_ranges.resize(inputs.size());

        std::generate(
            image_subresource_ranges.begin(),
            image_subresource_ranges.end(),
            generate_image_subresource_range
        );

        return image_subresource_ranges;
    }

    std::pmr::vector<vk::Rect2D> get_render_areas(
        std::span<Render_pipeline_input_configuration const> const inputs,
        std::span<Swapchain_configuration const> const swapchain_configurations,
        std::span<vk::Extent2D const> const surface_image_extents,
        std::pmr::polymorphic_allocator<> const& allocator
    )
    {
        auto const get_output_area = [=](Render_pipeline_input_configuration const& input) -> vk::Rect2D
        {
            return
            {
                .offset = {0, 0},
                .extent = surface_image_extents[swapchain_configurations[input.swapchain_index].surface_index],
            };
        };

        std::pmr::vector<vk::Rect2D> render_areas{ allocator };
        render_areas.resize(inputs.size());

        std::transform(
            inputs.begin(),
            inputs.end(),
            render_areas.begin(),
            get_output_area
        );

        return render_areas;
    }

    namespace
    {
        std::pmr::vector<std::pmr::vector<vk::ImageView>> create_render_pipeline_input_image_views(
            std::span<Render_pipeline_input_configuration const> const inputs,
            std::span<vk::Device const> const swapchain_devices,
            std::span<std::pmr::vector<vk::Image> const> swapchain_images,
            std::span<vk::Format const> swapchain_image_formats,
            std::span<vk::ImageSubresourceRange const> swapchain_image_subresource_ranges,
            std::pmr::polymorphic_allocator<> const& allocator
        )
        {
            auto const create_image_views = [=](Render_pipeline_input_configuration const& input) -> std::pmr::vector<vk::ImageView>
            {
                vk::Device const device = swapchain_devices[input.swapchain_index];
                std::span<vk::Image const> const images = swapchain_images[input.swapchain_index];
                vk::Format const format = swapchain_image_formats[input.swapchain_index];
                vk::ImageSubresourceRange const subresource_range = swapchain_image_subresource_ranges[input.swapchain_index];

                std::pmr::vector<vk::ImageView> image_views{ allocator };
                image_views.resize(images.size());

                for (std::size_t image_index = 0; image_index < images.size(); ++image_index)
                {
                    vk::ImageViewCreateInfo const create_info
                    {
                        .flags = {},
                        .image = images[image_index],
                        .viewType = vk::ImageViewType::e2D,
                        .format = format,
                        .components = {
                            .r = vk::ComponentSwizzle::eR,
                            .g = vk::ComponentSwizzle::eG,
                            .b = vk::ComponentSwizzle::eB,
                            .a = vk::ComponentSwizzle::eA,
                        },
                        .subresourceRange = subresource_range,
                    };

                    image_views[image_index] = device.createImageView(create_info);
                }

                return image_views;
            };

            std::pmr::vector<std::pmr::vector<vk::ImageView>> image_views{ allocator };
            image_views.resize(inputs.size());

            std::transform(
                inputs.begin(),
                inputs.end(),
                image_views.begin(),
                create_image_views
            );

            return image_views;
        }

        std::pmr::vector<std::pmr::vector<vk::Framebuffer>> create_render_pipeline_input_framebuffers(
            std::span<Render_pipeline_input_configuration const> const inputs,
            std::span<vk::Device const> const swapchain_devices,
            std::span<std::pmr::vector<vk::ImageView> const> const swapchain_image_views,
            std::span<vk::Rect2D const> const swapchain_render_areas,
            std::span<vk::RenderPass const> const render_pipeline_render_passes,
            std::pmr::polymorphic_allocator<> const& allocator
        )
        {
            auto const create_framebuffers = [=](Render_pipeline_input_configuration const& input) -> std::pmr::vector<vk::Framebuffer>
            {
                vk::Device const device = swapchain_devices[input.swapchain_index];
                std::span<vk::ImageView const> const image_views = swapchain_image_views[input.swapchain_index];
                vk::Extent2D const image_extent = swapchain_render_areas[input.swapchain_index].extent;

                std::pmr::vector<vk::Framebuffer> framebuffers{ allocator };
                framebuffers.resize(image_views.size());

                for (std::size_t image_view_index = 0; image_view_index < image_views.size(); ++image_view_index)
                {
                    vk::FramebufferCreateInfo const create_info
                    {
                        .flags = {},
                        .renderPass = render_pipeline_render_passes[input.framebuffer_render_pass_index],
                        .attachmentCount = 1,
                        .pAttachments = &image_views[image_view_index],
                        .width = image_extent.width,
                        .height = image_extent.height,
                        .layers = 1,
                    };

                    framebuffers[image_view_index] = device.createFramebuffer(create_info);
                }

                return framebuffers;
            };

            std::pmr::vector<std::pmr::vector<vk::Framebuffer>> framebuffers{ allocator };
            framebuffers.resize(inputs.size());

            std::transform(
                inputs.begin(),
                inputs.end(),
                framebuffers.begin(),
                create_framebuffers
            );

            return framebuffers;
        }
    }

    Render_pipeline_input_resources::Render_pipeline_input_resources(
        nlohmann::json const& descriptor_set_layouts_json,
        nlohmann::json const& frame_resources_json,
        std::span<Render_pipeline_input_configuration const> const inputs,
        std::span<vk::Device const> const swapchain_devices,
        std::span<std::pmr::vector<vk::Image> const> swapchain_images,
        std::span<vk::Format const> swapchain_image_formats,
        std::span<vk::ImageSubresourceRange const> swapchain_image_subresource_ranges,
        std::span<vk::Rect2D const> const swapchain_render_areas,
        vk::Device const render_pipeline_device,
        std::span<vk::RenderPass const> const render_pipeline_render_passes,
        std::span<vk::DescriptorSetLayout const> const render_pipeline_descriptor_set_layouts,
        std::uint32_t const frames_in_flight,
        vk::AllocationCallbacks const* const allocation_callbacks,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    ) :
        swapchain_devices{ swapchain_devices.begin(), swapchain_devices.end(), output_allocator },
        image_views{ create_render_pipeline_input_image_views(inputs, swapchain_devices, swapchain_images, swapchain_image_formats, swapchain_image_subresource_ranges, output_allocator) },
        framebuffers{ create_render_pipeline_input_framebuffers(inputs, swapchain_devices, this->image_views, swapchain_render_areas, render_pipeline_render_passes, output_allocator) },
        descriptor_pool{},
        descriptor_sets{}
    {
        if (frame_resources_json.contains("descriptor_sets"))
        {
            nlohmann::json const& descriptor_sets_json = frame_resources_json.at("descriptor_sets");

            if (descriptor_sets_json.contains("per_frame"))
            {
                nlohmann::json const& per_frame_descriptor_sets_json = descriptor_sets_json.at("per_frame");

                using namespace Maia::Renderer::Vulkan;

                this->descriptor_pool = create_descriptor_pool(
                    per_frame_descriptor_sets_json,
                    descriptor_set_layouts_json,
                    render_pipeline_device,
                    frames_in_flight,
                    allocation_callbacks,
                    temporaries_allocator
                );

                this->descriptor_sets = create_frame_descriptor_sets(
                    per_frame_descriptor_sets_json,
                    render_pipeline_device,
                    this->descriptor_pool,
                    render_pipeline_descriptor_set_layouts,
                    frames_in_flight,
                    output_allocator,
                    temporaries_allocator
                );

                this->descriptor_sets_bindings = create_descriptor_sets_bindings(
                    per_frame_descriptor_sets_json,
                    output_allocator
                );

                this->descriptor_sets_image_layouts = create_descriptor_sets_image_layouts(
                    per_frame_descriptor_sets_json,
                    output_allocator
                );

                std::pmr::vector<std::size_t> input_index_to_image_index; // TODO

                this->descriptor_sets_image_indices = create_descriptor_sets_image_indices(
                    per_frame_descriptor_sets_json,
                    input_index_to_image_index,
                    output_allocator
                );
            }
        }
    }

    Render_pipeline_input_resources::~Render_pipeline_input_resources() noexcept
    {
        for (std::size_t device_index = 0; device_index < this->swapchain_devices.size(); ++device_index)
        {
            vk::Device const device = this->swapchain_devices[device_index];

            for (vk::Framebuffer const framebuffer : this->framebuffers[device_index])
            {
                device.destroy(framebuffer);
            }
        }

        for (std::size_t device_index = 0; device_index < this->swapchain_devices.size(); ++device_index)
        {
            vk::Device const device = this->swapchain_devices[device_index];

            for (vk::ImageView const image_view : this->image_views[device_index])
            {
                device.destroy(image_view);
            }
        }
    }
}
