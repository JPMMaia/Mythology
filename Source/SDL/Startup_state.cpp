module;

#include <nlohmann/json.hpp>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <vulkan/vulkan.hpp>

#include <array>
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <memory_resource>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <variant>
#include <vector>

module mythology.sdl.startup_state;

import maia.glTF;
import maia.renderer.vulkan.serializer;
import maia.renderer.vulkan.upload;
import maia.scene;

import mythology.geometry;
import mythology.sdl.configuration;
import mythology.sdl.render_resources;
import mythology.sdl.sdl;
import mythology.sdl.state;
import mythology.sdl.vulkan;

namespace Mythology::SDL
{
    Startup_state::Startup_state(
        std::pmr::unordered_map<std::pmr::string, std::filesystem::path> render_pipelines,
        std::filesystem::path gltf_path
    ) noexcept :
        m_render_pipelines{ std::move(render_pipelines) },
        m_gltf_path{ std::move(gltf_path) }
    {
    }

    Startup_state::~Startup_state() noexcept
    {
    }

    namespace
    {
        nlohmann::json read_json_from_file(std::filesystem::path const& path)
        {
            std::ifstream input_stream{ path };
            assert(input_stream.good());

            nlohmann::json json{};
            input_stream >> json;

            return json;
        }

        struct Scene_resources
        {
            Maia::Scene::World world;
            std::pmr::vector<std::pmr::vector<std::byte>> buffers_data;
            std::pmr::vector<Mythology::Acceleration_structure> bottom_level_acceleration_structures;
            std::pmr::vector<Mythology::Acceleration_structure> top_level_acceleration_structures;
        };

        struct Buffer_resources
        {
            Maia::Renderer::Vulkan::Buffer_resources shader_binding_tables;
            Maia::Renderer::Vulkan::Buffer_resources acceleration_structure_storage;
            Maia::Renderer::Vulkan::Buffer_resources geometry;
            Maia::Renderer::Vulkan::Buffer_resources instance;
            Maia::Renderer::Vulkan::Buffer_resources upload;
            Maia::Renderer::Vulkan::Buffer_resources scratch;
        };

        Buffer_resources create_buffer_resources(
            vk::PhysicalDevice const physical_device,
            vk::PhysicalDeviceType const physical_device_type,
            vk::Device const device
        )
        {
            return Buffer_resources
            {
                .shader_binding_tables =
                {
                    physical_device,
                    device,
                    physical_device_type,
                    {},
                    4 * 1024,
                    vk::BufferUsageFlagBits::eShaderBindingTableKHR,
                    {},
                    {},
                    nullptr,
                    {}
                },
                .acceleration_structure_storage =
                {
                    physical_device,
                    device,
                    physical_device_type,
                    {},
                    64 * 1024 * 1024,
                    vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR,
                    {},
                    {},
                    nullptr,
                    {}
                },
                .geometry =
                {
                    physical_device,
                    device,
                    physical_device_type,
                    {},
                    64 * 1024 * 1024,
                    vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddressKHR,
                    {},
                    {},
                    nullptr,
                    {}
                },
                .instance =
                {
                    physical_device,
                    device,
                    physical_device_type,
                    {},
                    64 * 1024 * 1024,
                    vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddressKHR,
                    {},
                    {},
                    nullptr,
                    {}
                },
                .upload =
                {
                    physical_device,
                    device,
                    physical_device_type,
                    {},
                    64 * 1024 * 1024,
                    vk::BufferUsageFlagBits::eTransferSrc,
                    {},
                    {},
                    nullptr,
                    {}
                },
                .scratch
                {
                    physical_device,
                    device,
                    physical_device_type,
                    {},
                    64 * 1024 * 1024,
                    vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddressKHR,
                    {},
                    {},
                    nullptr,
                    {}
                },
            };
        }

        std::optional<Scene_resources> load_scene(
            std::filesystem::path const& gltf_path,
            vk::PhysicalDeviceType physical_device_type,
            vk::Device device,
            vk::Queue queue,
            vk::CommandPool command_pool,
            Buffer_resources& buffer_resources,
            vk::AllocationCallbacks const* allocation_callbacks,
            std::pmr::polymorphic_allocator<> const& output_allocator,
            std::pmr::polymorphic_allocator<> const& temporaries_allocator
        )
        {
            nlohmann::json const gltf_json = read_json_from_file(gltf_path);
            Maia::Scene::World world = Maia::glTF::gltf_from_json(gltf_json, output_allocator);

            if (!world.scene_index.has_value())
            {
                return {};
            }

            Maia::Scene::Scene const& scene = world.scenes[*world.scene_index];
            std::pmr::vector<std::pmr::vector<std::byte>> buffers_data = Maia::Scene::read_buffers_data(world, gltf_path.parent_path(), output_allocator);

            std::pmr::vector<Mythology::Acceleration_structure> bottom_level_acceleration_structures = create_bottom_level_acceleration_structures(
                physical_device_type,
                device,
                queue,
                command_pool,
                buffer_resources.acceleration_structure_storage,
                buffer_resources.geometry,
                buffer_resources.upload,
                buffer_resources.scratch,
                world,
                buffers_data,
                allocation_callbacks,
                output_allocator,
                temporaries_allocator
            );

            std::pmr::vector<Mythology::Acceleration_structure> top_level_acceleration_structures = create_top_level_acceleration_structures(
                physical_device_type,
                device,
                queue,
                command_pool,
                bottom_level_acceleration_structures,
                buffer_resources.acceleration_structure_storage,
                buffer_resources.instance,
                buffer_resources.upload,
                buffer_resources.scratch,
                world,
                scene,
                allocation_callbacks,
                output_allocator,
                temporaries_allocator
            );

            return Scene_resources
            {
                .world = std::move(world),
                .buffers_data = std::move(buffers_data),
                .bottom_level_acceleration_structures = std::move(bottom_level_acceleration_structures),
                .top_level_acceleration_structures = std::move(top_level_acceleration_structures)
            };
        }

        vk::PhysicalDeviceRayTracingPipelinePropertiesKHR get_physical_device_ray_tracing_properties(vk::PhysicalDevice const physical_device) noexcept
        {
            vk::PhysicalDeviceRayTracingPipelinePropertiesKHR ray_tracing_properties = {};
            vk::PhysicalDeviceProperties2 properties = {
                .pNext = &ray_tracing_properties,
            };

            physical_device.getProperties2(&properties);

            return ray_tracing_properties;
        }
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
                    "VK_KHR_acceleration_structure",
                    "VK_KHR_buffer_device_address",
                    "VK_KHR_deferred_host_operations",
                    "VK_KHR_ray_tracing_pipeline",
                    "VK_KHR_swapchain",
                },
                .upload_queue_index = 0,
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

        SDL_instance sdl{ SDL_INIT_VIDEO };

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
            throw std::runtime_error{ "Swapchain present queues are not compatible!" };
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
        Render_pipeline_configuration const& render_pipeline_configuration = render_pipeline_configurations[render_pipeline_index];

        std::filesystem::path const render_pipeline_configuration_file_path =
            m_render_pipelines.at(render_pipeline_configuration.name);

        nlohmann::json const render_pipeline_json = read_json_from_file(render_pipeline_configuration_file_path);

        vk::Device const render_pipeline_device = device_resources.devices[render_pipeline_configuration.device_index];
        vk::Queue const render_pipeline_upload_queue = queues[device_configurations[render_pipeline_configuration.device_index].upload_queue_index];
        vk::CommandPool const render_pipeline_command_pool = command_pools_resources.command_pools[device_configurations[render_pipeline_configuration.device_index].upload_queue_index];
        vk::PhysicalDevice const render_pipeline_physical_device = physical_devices[device_configurations[render_pipeline_configuration.device_index].physical_device_index];
        vk::PhysicalDeviceType const render_pipeline_physical_device_type = render_pipeline_physical_device.getProperties().deviceType;

        Buffer_resources buffer_resources = create_buffer_resources(
            render_pipeline_physical_device,
            render_pipeline_physical_device_type,
            render_pipeline_device
        );

        std::unique_ptr<Maia::Renderer::Vulkan::Upload_buffer> render_pipeline_upload_buffer =
            (render_pipeline_physical_device_type != vk::PhysicalDeviceType::eIntegratedGpu) ?
            std::make_unique<Maia::Renderer::Vulkan::Upload_buffer>(render_pipeline_device, buffer_resources.upload, 64 * 1024 * 1024) :
            nullptr;

        vk::PhysicalDeviceRayTracingPipelinePropertiesKHR const physical_device_ray_tracing_properties = get_physical_device_ray_tracing_properties(render_pipeline_physical_device);

        Maia::Renderer::Vulkan::Pipeline_resources const render_pipeline_resources
        {
            render_pipeline_physical_device,
            render_pipeline_physical_device_type,
            render_pipeline_device,
            render_pipeline_upload_queue,
            render_pipeline_command_pool,
            {},
            physical_device_ray_tracing_properties,
            buffer_resources.shader_binding_tables,
            render_pipeline_upload_buffer.get(),
            nullptr,
            render_pipeline_json,
            render_pipeline_configuration_file_path.parent_path(),
            {},
            {}
        };

        std::pmr::vector<vk::ImageSubresourceRange> const swapchain_image_subresource_ranges =
            get_image_subresource_ranges(
                render_pipeline_configuration.inputs,
                {}
        );

        std::pmr::vector<vk::Rect2D> const swapchain_render_areas =
            get_render_areas(
                render_pipeline_configuration.inputs,
                swapchain_configurations,
                surface_image_extents,
                {}
        );

        std::pmr::vector<vk::Format> const swapchain_formats = get_swapchain_image_formats(swapchain_configurations, {});

        Mythology::SDL::Render_pipeline_input_resources const pipeline_input_resources
        {
            render_pipeline_configuration.inputs,
            swapchain_devices,
            swapchain_resources.images,
            swapchain_formats,
            swapchain_image_subresource_ranges,
            swapchain_render_areas,
            render_pipeline_resources.render_passes,
            {}
        };

        nlohmann::json const& command_lists_json = render_pipeline_json.at("frame_commands");
        nlohmann::json const& command_list_json = command_lists_json[render_pipeline_configuration.command_list_index];

        Maia::Renderer::Vulkan::Commands_data const commands_data =
            Maia::Renderer::Vulkan::create_commands_data(
                command_list_json,
                render_pipeline_resources.pipeline_states,
                render_pipeline_resources.render_passes,
                render_pipeline_resources.shader_binding_tables,
                {},
                {}
        );

        std::optional<Scene_resources> const scene_resources = load_scene(
            m_gltf_path,
            render_pipeline_physical_device_type,
            render_pipeline_device,
            render_pipeline_upload_queue,
            render_pipeline_command_pool,
            buffer_resources,
            nullptr,
            {},
            {}
        );

        if (!scene_resources)
        {
            return {};
        }

        std::uint8_t frame_index = 0;

        while (true)
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
                            std::span<vk::Buffer const> const output_buffers; // TODO

                            std::array<std::uint32_t, 1> const swapchain_image_indices = { swapchain_image_index };
                            std::pmr::vector<vk::Image> const output_images =
                                get_input_images(
                                    render_pipeline_configuration.inputs,
                                    swapchain_resources.images,
                                    swapchain_image_indices,
                                    {}
                            );

                            std::pmr::vector<vk::ImageView> const output_image_views =
                                get_input_image_views(
                                    render_pipeline_configuration.inputs,
                                    pipeline_input_resources.image_views,
                                    swapchain_image_indices,
                                    {}
                            );

                            std::span<vk::ImageSubresourceRange const> const output_image_subresource_ranges =
                                swapchain_image_subresource_ranges;

                            std::pmr::vector<vk::Framebuffer> const output_framebuffers =
                                get_input_framebuffers(
                                    render_pipeline_configuration.inputs,
                                    pipeline_input_resources.framebuffers,
                                    swapchain_image_indices,
                                    {}
                            );

                            std::span<vk::Rect2D const> const output_render_areas = swapchain_render_areas;

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
                            std::array<vk::PipelineStageFlags, 1> constexpr wait_destination_stage_masks = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
                            std::array<vk::Fence, 1> const fences_to_reset = { available_frame_fence };
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
                            std::array<std::uint32_t, 1> const swapchain_image_indices = { swapchain_image_index };

                            vk::PresentInfoKHR const present_info
                            {
                                .waitSemaphoreCount = 1,
                                .pWaitSemaphores = &finished_frame_semaphore,
                                .swapchainCount = 1,
                                .pSwapchains = &swapchain,
                                .pImageIndices = swapchain_image_indices.data(),
                                .pResults = nullptr,
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
