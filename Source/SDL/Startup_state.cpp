module;

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <vulkan/vulkan.hpp>

#include <array>
#include <cassert>
#include <memory>
#include <memory_resource>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <variant>
#include <vector>

module mythology.sdl.startup_state;

import maia.renderer.vulkan.serializer;

import mythology.sdl.configuration;
import mythology.sdl.render_resources;
import mythology.sdl.sdl;
import mythology.sdl.state;
import mythology.sdl.vulkan;

namespace Mythology::SDL
{
    Startup_state::Startup_state(
        std::pmr::unordered_map<std::pmr::string, std::filesystem::path> render_pipelines
    ) noexcept :
        m_render_pipelines{std::move(render_pipelines)}
    {
    }

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
            },
            /*{
                .vendor_ID = 0x8086,
                .device_ID = 0x3e9b,
            }*/
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
                .image_format = vk::Format::eB8G8R8A8Srgb,
            }
        };

        std::array<Render_pipeline_configuration, 1> const render_pipeline_configurations
        {
            Render_pipeline_configuration
            {
                .name = "default",
                .command_list_index = 0,
                .inputs
                {
                    Render_pipeline_input_configuration
                    {
                        .swapchain_index = 0
                    }
                },
            }
        };

        SDL_instance sdl{SDL_INIT_VIDEO};

        std::pmr::vector<SDL_window> const windows = create_windows(sdl, window_configurations);

        std::pmr::vector<char const*> const required_instance_extensions =
            Mythology::SDL::Vulkan::get_sdl_required_instance_extensions({});

        vk::DynamicLoader dynamic_loader;
        PFN_vkGetInstanceProcAddr const get_instance_proccess_address = Mythology::SDL::Vulkan::get_instance_process_address();
        VULKAN_HPP_DEFAULT_DISPATCHER.init(get_instance_proccess_address);

        Mythology::Render::Instance_resources instance_resources
        {
            VK_MAKE_VERSION(1, 2, 0),
            required_instance_extensions
        };
        VULKAN_HPP_DEFAULT_DISPATCHER.init(instance_resources.instance);

        std::pmr::vector<SDL_Window*> const surface_windows =
            select_surface_windows(surface_configurations, windows, {});

        Mythology::SDL::Vulkan::Surface_resources surface_resources
        {
            instance_resources.instance,
            surface_windows,
            {}
        };

        std::pmr::vector<vk::PhysicalDevice> const physical_devices = get_physical_devices(
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

        std::pmr::vector<vk::Queue> const queues = get_queues(
            queue_configurations,
            device_resources.devices,
            {}
        );

        std::pmr::vector<vk::Device> const swapchain_devices = get_swapchain_devices(
            swapchain_configurations,
            device_resources.devices,
            {}
        );

        std::pmr::vector<SDL_Window*> const window_raw_pointers = get_window_raw_pointers(
            windows,
            {}
        );

        std::pmr::vector<vk::Extent2D> const surface_image_extents = get_image_extents(
            surface_configurations,
            window_raw_pointers,
            {}
        );

        if (!validate_swapchain_queues(device_configurations, queue_configurations, swapchain_configurations, physical_devices, surface_resources.surfaces))
        {
            throw std::runtime_error{"Swapchain present queues are not compatible!"};
        }

        std::pmr::vector<vk::SurfaceCapabilitiesKHR> const swapchain_surface_capabilities = get_swapchain_surface_capabilities(
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

        std::pmr::vector<vk::Device> const queue_devices = get_queue_devices(queue_configurations, device_resources.devices, {});
        std::pmr::vector<vk::CommandPoolCreateFlags> const command_pool_flags(queue_configurations.size(), vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
        std::pmr::vector<std::uint32_t> command_pools_queue_family_indices = get_queue_family_indices(queue_configurations, {});

        Mythology::Render::Command_pools_resources const command_pools_resources
        {
            queue_devices,
            command_pool_flags,
            command_pools_queue_family_indices,
            {}
        };

        std::pmr::vector<std::pmr::vector<vk::CommandBuffer>> const frames_command_buffers = 
            Mythology::Render::allocate_command_buffers(
                queue_devices,
                command_pools_resources.command_pools,
                vk::CommandBufferLevel::ePrimary,
                number_of_frames_in_flight,
                nullptr,
                {}
            );

        std::size_t const render_pipeline_index = 0;

        std::uint8_t frame_index = 0;

        {
            std::span<vk::Fence const> const available_frame_fences = synchronization_resources.frames[frame_index].available_frame_fences;
            std::span<vk::Semaphore const> const available_frame_semaphores = synchronization_resources.frames[frame_index].available_frame_semaphores;
            std::span<vk::Semaphore const> const finished_frame_semaphores = synchronization_resources.frames[frame_index].finished_frame_semaphores;
            std::span<vk::CommandBuffer const> const frame_command_buffers = frames_command_buffers[frame_index];

            {
                Render_pipeline_configuration const& render_pipeline_configuration = render_pipeline_configurations[render_pipeline_index];
                std::size_t const swapchain_index = render_pipeline_configuration.inputs[0].swapchain_index; // TODO

                vk::Device const swapchain_device = synchronization_resources.devices[swapchain_index];
                vk::Fence const available_frame_fence = available_frame_fences[swapchain_index];

                bool const is_fence_signaled = swapchain_device.getFenceStatus(available_frame_fence) == vk::Result::eSuccess;
                
                if (is_fence_signaled)
                {
                    vk::SwapchainKHR const swapchain = swapchain_resources.swapchains[swapchain_index];

                    vk::Semaphore const available_frame_semaphore = available_frame_semaphores[swapchain_index];

                    vk::ResultValue<std::uint32_t> const acquire_next_image_result =
                        swapchain_device.acquireNextImageKHR(swapchain, 0, available_frame_semaphore, {});

                    if (acquire_next_image_result.result == vk::Result::eSuccess)
                    {
                        std::uint32_t const swapchain_image_index = acquire_next_image_result.value;

                        vk::CommandBuffer const command_buffer = frame_command_buffers[0];
                        command_buffer.reset({});
                        {
                            vk::CommandBufferBeginInfo const begin_info
                            {
                                .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
                                .pInheritanceInfo = nullptr,
                            };

                            command_buffer.begin(begin_info);
                        }

                        {
                            // TODO
                            std::span<vk::Buffer const> const output_buffers;
                            
                            // TODO
                            std::array<std::uint32_t, 1> const swapchain_image_indices = {swapchain_image_index};
                            std::pmr::vector<vk::Image> const output_images =
                                get_input_images(
                                    render_pipeline_configuration.inputs,
                                    swapchain_resources.images,
                                    swapchain_image_indices,
                                    {}
                                );
                            
                            std::span<vk::ImageView const> const output_image_views;
                            
                            std::pmr::vector<vk::ImageSubresourceRange> const output_image_subresource_ranges =
                                get_image_subresource_ranges(
                                    render_pipeline_configuration.inputs,
                                    {}
                                );
                            
                            std::span<vk::Framebuffer const> const output_framebuffers;
                            
                            std::pmr::vector<vk::Rect2D> const output_render_areas =
                                get_render_areas(
                                    render_pipeline_configuration.inputs,
                                    swapchain_configurations,
                                    surface_image_extents,
                                    {}
                                );

                            Maia::Renderer::Vulkan::Commands_data commands_data; // TODO

                            Maia::Renderer::Vulkan::draw(
                                command_buffer,
                                output_buffers,
                                output_images,
                                output_image_views,
                                output_image_subresource_ranges,
                                output_framebuffers,
                                output_render_areas,
                                commands_data,
                                {}
                            );
                        }
                        command_buffer.end();

                        vk::Queue const queue = queues[swapchain_configurations[swapchain_index].queue_to_present_index];

                        vk::Semaphore const finished_frame_semaphore = finished_frame_semaphores[swapchain_index];
                        {
                            std::array<vk::PipelineStageFlags, 1> constexpr wait_destination_stage_masks = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
                            std::array<vk::Fence, 1> const fences_to_reset = {available_frame_fence};
                            swapchain_device.resetFences(fences_to_reset);

                            std::array<vk::SubmitInfo, 1> const submit_infos
                            {
                                vk::SubmitInfo
                                {
                                    .waitSemaphoreCount = 1,
                                    .pWaitSemaphores = &available_frame_semaphore,
                                    .pWaitDstStageMask = wait_destination_stage_masks.data(),
                                    .commandBufferCount = static_cast<std::uint32_t>(frame_command_buffers.size()),
                                    .pCommandBuffers = frame_command_buffers.data(),
                                    .signalSemaphoreCount = 1,
                                    .pSignalSemaphores = &finished_frame_semaphore,
                                }
                            };

                            queue.submit(submit_infos, available_frame_fence);
                        }

                        {
                            std::array<std::uint32_t, 1> const swapchain_image_indices = {swapchain_image_index};
                            
                            vk::PresentInfoKHR const present_info
                            {
                                .waitSemaphoreCount = 1,
                                .pWaitSemaphores    = &finished_frame_semaphore,
                                .swapchainCount     = 1,
                                .pSwapchains        = &swapchain,
                                .pImageIndices      = swapchain_image_indices.data(),
                                .pResults           = nullptr,
                            };

                            vk::Result const result = queue.presentKHR(present_info);

                            if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
                            {
                                // TODO recreate swapchain
                            }
                        }
                    }
                    else if (acquire_next_image_result.result == vk::Result::eErrorOutOfDateKHR
                          || acquire_next_image_result.result == vk::Result::eSuboptimalKHR)
                    {
                        // TODO recreate swapchain
                    }
                }
            }
        }

        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(5s);
        }

        for (vk::Device const device : device_resources.devices)
        {
            device.waitIdle();
        }

        // Load loading assets
        // Render black screen while loading assets

        // Return next state
        return {};
    }
}
