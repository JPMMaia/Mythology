module;

#include <nlohmann/json.hpp>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <vulkan/vulkan.hpp>

#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
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

import mythology.addon_interface;
import mythology.load_addon;
import mythology.geometry;
import mythology.sdl.configuration;
import mythology.sdl.input;
import mythology.sdl.render_resources;
import mythology.sdl.sdl;
import mythology.sdl.state;
import mythology.sdl.vulkan;

namespace Mythology::SDL
{
    Startup_state::Startup_state(
        std::pmr::unordered_map<std::pmr::string, std::filesystem::path> render_pipelines,
        std::filesystem::path gltf_path,
        std::span<std::filesystem::path const> const addon_paths
    ) noexcept :
        m_render_pipelines{ std::move(render_pipelines) },
        m_gltf_path{ std::move(gltf_path) },
        m_addon_paths{ addon_paths }
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

        void check_ray_tracing_support(std::span<vk::PhysicalDevice const> const physical_devices)
        {
            auto const supports_ray_tracing = [](vk::PhysicalDevice const physical_device) -> bool
            {
                vk::PhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_features{};

                vk::PhysicalDeviceFeatures2 features
                {
                    .pNext = &ray_tracing_features,
                };

                physical_device.getFeatures2(&features);

                return ray_tracing_features.rayTracingPipeline;
            };

            for (vk::PhysicalDevice const physical_device : physical_devices)
            {
                vk::PhysicalDeviceProperties const properties = physical_device.getProperties();

                if (supports_ray_tracing(physical_device))
                {
                    std::cout << std::format("Physical device {} supports ray tracing.\n", properties.deviceName.data());
                }
            }
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
                    vk::MemoryAllocateFlagBits::eDeviceAddress,
                    4 * 1024,
                    vk::BufferUsageFlagBits::eShaderBindingTableKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eTransferDst,
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
                    vk::MemoryAllocateFlagBits::eDeviceAddress,
                    64 * 1024 * 1024,
                    vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddressKHR | vk::BufferUsageFlagBits::eTransferDst,
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
                    vk::MemoryAllocateFlagBits::eDeviceAddress,
                    64 * 1024 * 1024,
                    vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddressKHR | vk::BufferUsageFlagBits::eTransferDst,
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
                    vk::MemoryAllocateFlagBits::eDeviceAddress,
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
            vk::Device device,
            vk::Queue queue,
            vk::CommandPool command_pool,
            Maia::Renderer::Vulkan::Upload_buffer const* upload_buffer,
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
                device,
                queue,
                command_pool,
                buffer_resources.acceleration_structure_storage,
                buffer_resources.geometry,
                buffer_resources.scratch,
                upload_buffer,
                world,
                buffers_data,
                allocation_callbacks,
                output_allocator,
                temporaries_allocator
            );

            std::pmr::vector<Mythology::Acceleration_structure> top_level_acceleration_structures = create_top_level_acceleration_structures(
                device,
                queue,
                command_pool,
                bottom_level_acceleration_structures,
                buffer_resources.acceleration_structure_storage,
                buffer_resources.instance,
                buffer_resources.scratch,
                upload_buffer,
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

        std::pmr::vector<vk::AccelerationStructureKHR> get_acceleration_structure_handles(
            std::span<Mythology::Acceleration_structure const> const acceleration_structures,
            std::pmr::polymorphic_allocator<> const& output_allocator
        )
        {
            std::pmr::vector<vk::AccelerationStructureKHR> handles{ output_allocator };
            handles.resize(acceleration_structures.size());

            std::transform(
                acceleration_structures.begin(),
                acceleration_structures.end(),
                handles.begin(),
                [](Mythology::Acceleration_structure const& acceleration_structure)->vk::AccelerationStructureKHR { return acceleration_structure.handle; }
            );

            return handles;
        }

        struct Frame_shared_resources
        {
            Frame_shared_resources(
                vk::Device const device,
                nlohmann::json const& frame_shared_descriptor_sets_json,
                nlohmann::json const& descriptor_set_layouts_json,
                std::span<vk::DescriptorSetLayout const> const descriptor_set_layouts,
                std::span<vk::AccelerationStructureKHR const> const top_level_acceleration_structures,
                vk::AllocationCallbacks const* const allocation_callbacks,
                std::pmr::polymorphic_allocator<> const& output_allocator,
                std::pmr::polymorphic_allocator<> const& temporaries_allocator
            ) :
                device{ device },
                allocation_callbacks{ allocation_callbacks }
            {
                this->descriptor_pool = Maia::Renderer::Vulkan::create_descriptor_pool(
                    frame_shared_descriptor_sets_json,
                    descriptor_set_layouts_json,
                    device,
                    1,
                    allocation_callbacks,
                    temporaries_allocator
                );

                this->descriptor_sets = Maia::Renderer::Vulkan::create_descriptor_sets(
                    frame_shared_descriptor_sets_json,
                    device,
                    this->descriptor_pool,
                    descriptor_set_layouts,
                    output_allocator,
                    temporaries_allocator
                );

                Maia::Renderer::Vulkan::update_descriptor_sets(
                    frame_shared_descriptor_sets_json,
                    device,
                    this->descriptor_sets,
                    {},
                    {},
                    {},
                    {},
                    top_level_acceleration_structures,
                    temporaries_allocator
                );
            }

            ~Frame_shared_resources()
            {
                this->device.destroy(this->descriptor_pool, this->allocation_callbacks);
            }

            vk::Device device = {};
            vk::DescriptorPool descriptor_pool = {};
            std::pmr::vector<vk::DescriptorSet> descriptor_sets;
            vk::AllocationCallbacks const* allocation_callbacks = {};
        };

        vk::PhysicalDeviceRayTracingPipelinePropertiesKHR get_physical_device_ray_tracing_properties(vk::PhysicalDevice const physical_device) noexcept
        {
            vk::PhysicalDeviceRayTracingPipelinePropertiesKHR ray_tracing_properties = {};
            vk::PhysicalDeviceProperties2 properties = {
                .pNext = &ray_tracing_properties,
            };

            physical_device.getProperties2(&properties);

            return ray_tracing_properties;
        }

        bool is_fence_signaled(
            Mythology::Render::Synchronization_resources const& synchronization_resources,
            std::size_t const frame_index,
            std::size_t const swapchain_index
        )
        {
            vk::Device const swapchain_device = synchronization_resources.devices[swapchain_index];

            std::span<vk::Fence const> const available_frame_fences = synchronization_resources.frames[frame_index].available_frame_fences;
            vk::Fence const available_frame_fence = available_frame_fences[swapchain_index];

            vk::Result const fence_status = swapchain_device.getFenceStatus(available_frame_fence);

            return fence_status == vk::Result::eSuccess;
        }

        std::optional<std::uint32_t> acquire_next_image(
            Mythology::Render::Synchronization_resources const& synchronization_resources,
            std::size_t const frame_index,
            Mythology::SDL::Swapchain_resources const& swapchain_resources,
            std::size_t const swapchain_index
        )
        {
            vk::Device const swapchain_device = synchronization_resources.devices[swapchain_index];
            vk::SwapchainKHR const swapchain = swapchain_resources.swapchains[swapchain_index];

            std::span<vk::Semaphore const> const available_frame_semaphores = synchronization_resources.frames[frame_index].available_frame_semaphores;
            vk::Semaphore const available_frame_semaphore = available_frame_semaphores[swapchain_index];

            vk::ResultValue<std::uint32_t> const acquire_next_image_result =
                swapchain_device.acquireNextImageKHR(swapchain, 0, available_frame_semaphore, {});

            if (acquire_next_image_result.result == vk::Result::eSuccess)
            {
                return acquire_next_image_result.value;
            }
            else
            {
                return std::nullopt;
            }
        }

        void render_frame(
            vk::Device const render_pipeline_device,
            std::span<Render_pipeline_input_configuration const> const render_pipeline_inputs,
            Mythology::SDL::Render_pipeline_input_resources const& render_pipeline_input_resources,
            std::span<vk::Queue const> const queues,
            Frame_shared_resources const& frame_shared_resources,
            std::size_t frame_index,
            std::span<vk::CommandBuffer const> const frame_command_buffers,
            Maia::Renderer::Vulkan::Frame_descriptor_sets_map const& frame_descriptor_sets_map,
            Mythology::Render::Synchronization_resources const& synchronization_resources,
            std::span<Swapchain_configuration const> const swapchain_configurations,
            Mythology::SDL::Swapchain_resources const& swapchain_resources,
            std::span<vk::Rect2D const> const swapchain_render_areas,
            std::span<vk::ImageSubresourceRange const> const swapchain_image_subresource_ranges,
            std::size_t const swapchain_index,
            std::uint32_t const swapchain_image_index,
            std::span<std::pmr::vector<std::byte> const> const data_arrays,
            Maia::Renderer::Vulkan::Commands_data const& commands_data
        )
        {
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
                std::span<Maia::Renderer::Vulkan::Buffer_view const> const output_buffer_memory_views; // TODO

                std::array<std::uint32_t, 1> const swapchain_image_indices = { swapchain_image_index };
                std::pmr::vector<vk::Image> const output_images =
                    get_input_images(
                        render_pipeline_inputs,
                        swapchain_resources.images,
                        swapchain_image_indices,
                        {}
                );

                std::pmr::vector<vk::ImageView> const output_image_views =
                    get_input_image_views(
                        render_pipeline_inputs,
                        render_pipeline_input_resources.image_views,
                        swapchain_image_indices,
                        {}
                );

                std::span<vk::ImageSubresourceRange const> const output_image_subresource_ranges =
                    swapchain_image_subresource_ranges;

                std::pmr::vector<vk::Framebuffer> const output_framebuffers =
                    get_input_framebuffers(
                        render_pipeline_inputs,
                        render_pipeline_input_resources.framebuffers,
                        swapchain_image_indices,
                        {}
                );

                std::span<vk::Rect2D const> const output_render_areas = swapchain_render_areas;

                if (!render_pipeline_input_resources.descriptor_sets.empty() && !render_pipeline_input_resources.descriptor_sets[frame_index].empty())
                {
                    Maia::Renderer::Vulkan::update_frame_descriptor_sets(
                        render_pipeline_device,
                        render_pipeline_input_resources.descriptor_sets[frame_index],
                        render_pipeline_input_resources.descriptor_sets_image_indices,
                        render_pipeline_input_resources.descriptor_sets_image_layouts,
                        output_image_views,
                        render_pipeline_input_resources.descriptor_sets_bindings,
                        {}
                    );
                }

                std::span<vk::DescriptorSet const> const per_frame_descriptor_sets
                {
                    !render_pipeline_input_resources.descriptor_sets.empty() ? render_pipeline_input_resources.descriptor_sets[frame_index].data() : nullptr,
                    !render_pipeline_input_resources.descriptor_sets.empty() ? render_pipeline_input_resources.descriptor_sets[frame_index].size() : 0
                };

                std::pmr::vector<vk::DescriptorSet> const frame_descriptor_sets =
                    Maia::Renderer::Vulkan::get_frame_descriptor_sets(
                        per_frame_descriptor_sets,
                        frame_shared_resources.descriptor_sets,
                        frame_descriptor_sets_map,
                        {}
                );

                Maia::Renderer::Vulkan::draw(
                    command_buffer,
                    data_arrays,
                    output_buffer_memory_views,
                    output_images,
                    output_image_views,
                    output_image_subresource_ranges,
                    frame_descriptor_sets,
                    output_framebuffers,
                    output_render_areas,
                    commands_data,
                    {}
                );
            }
            command_buffer.end();

            vk::Queue const queue = queues[swapchain_configurations[swapchain_index].queue_to_present_index];

            std::span<vk::Semaphore const> const finished_frame_semaphores = synchronization_resources.frames[frame_index].finished_frame_semaphores;
            vk::Semaphore const finished_frame_semaphore = finished_frame_semaphores[swapchain_index];
            {
                std::span<vk::Fence const> const available_frame_fences = synchronization_resources.frames[frame_index].available_frame_fences;
                vk::Fence const available_frame_fence = available_frame_fences[swapchain_index];

                std::span<vk::Semaphore const> const available_frame_semaphores = synchronization_resources.frames[frame_index].available_frame_semaphores;
                vk::Semaphore const available_frame_semaphore = available_frame_semaphores[swapchain_index];

                vk::Device const swapchain_device = synchronization_resources.devices[swapchain_index];

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
                vk::SwapchainKHR const swapchain = swapchain_resources.swapchains[swapchain_index];

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

        bool process_window_events(
            std::function<void()> recreate_swapchain,
            std::pmr::vector<Game_controller>& game_controllers,
            std::function<void(Sint32, std::pmr::vector<Game_controller>&)> add_game_controller,
            std::function<void(Sint32, std::pmr::vector<Game_controller>&)> remove_game_controller
        )
        {
            SDL_Event event = {};
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                {
                    return false;
                }
                else if (event.type == SDL_WINDOWEVENT)
                {
                    if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED || event.window.event == SDL_WINDOWEVENT_RESIZED)
                    {
                        recreate_swapchain();
                    }
                }
                else if (event.type == SDL_CONTROLLERDEVICEADDED)
                {
                    Sint32 const added_instance_id = event.cdevice.which;
                    add_game_controller(added_instance_id, game_controllers);
                }
                else if (event.type == SDL_CONTROLLERDEVICEREMOVED)
                {
                    Sint32 const removed_instance_id = event.cdevice.which;
                    remove_game_controller(removed_instance_id, game_controllers);
                }
            }

            return true;
        }

        void process_input(
            std::span<Addon_interface* const> const addons,
            std::span<Game_controller const> const game_controllers,
            std::pmr::polymorphic_allocator<> const& temporaries_allocator
        )
        {
            Maia::Input::Keyboard_state const current_keyboard_state = get_keyboard_state();
            Maia::Input::Mouse_state const current_mouse_state = get_mouse_state();
            std::pmr::vector<Maia::Input::Game_controller_state> const current_game_controllers_state =
                get_game_controllers_state(game_controllers, temporaries_allocator);

            for (Addon_interface* const addon : addons)
            {
                addon->process_input(
                    current_keyboard_state,
                    current_mouse_state,
                    current_game_controllers_state
                );
            }
        }

        void fixed_update(
            std::span<Addon_interface* const> const addons
        )
        {
            for (Addon_interface* const addon : addons)
            {
                addon->fixed_update();
            }
        }

        std::pmr::vector<std::pair<Addon, Addon_interface_pointer>> load_addons(
            std::span<std::filesystem::path const> const addon_paths,
            std::pmr::polymorphic_allocator<> const& output_allocator
        )
        {
            std::pmr::vector<std::pair<Addon, Addon_interface_pointer>> addons{ output_allocator };
            addons.reserve(addon_paths.size());

            for (std::filesystem::path const& addon_path : addon_paths)
            {
                std::optional<Addon> addon = Mythology::load_addon(addon_path);

                if (addon)
                {
                    Addon_interface_pointer addon_interface_pointer = Mythology::create_addon_interface(
                        addon->create_addon_interface,
                        addon->destroy_addon_interface
                    );

                    addons.push_back(std::make_pair(std::move(*addon), std::move(addon_interface_pointer)));
                }
            }

            return addons;
        }

        std::pmr::vector<Addon_interface*> get_addon_interfaces(
            std::span<std::pair<Addon, Addon_interface_pointer> const> const addons,
            std::pmr::polymorphic_allocator<> const& output_allocator
        )
        {
            std::pmr::vector<Addon_interface*> output{ output_allocator };
            output.resize(addons.size());

            std::transform(
                addons.begin(),
                addons.end(),
                output.begin(),
                [](std::pair<Addon, Addon_interface_pointer> const& addon) -> Addon_interface* { return addon.second.get(); }
            );

            return output;
        }
    }

    std::unique_ptr<State> Startup_state::run()
    {
        std::size_t const number_of_frames_in_flight = 3;

        Instance_configuration const instance_configuration
        {
            .vulkan_version = Vulkan_version(1, 2, 0),
            .enabled_extensions =
            {
                VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
            }
        };

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
                    VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME,
                    "VK_KHR_deferred_host_operations",
                    "VK_KHR_ray_tracing_pipeline",
                    VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
                    "VK_KHR_swapchain",
                },
                .upload_queue_index = 0,
                .vulkan_12_features = Physical_device_vulkan_12_features
                {
                    .buffer_device_address = true,
                },
                .acceleration_structure_features = Physical_device_acceleration_structure_features
                {
                    .acceleration_structure = true,
                },
                .ray_tracing_features = Ray_tracing_features_configuration
                {
                    .ray_tracing_pipeline = true,
                    .ray_tracing_pipeline_shader_group_handle_capture_replay = false,
                    .ray_tracing_pipeline_shader_group_handle_capture_replay_mixed = false,
                    .ray_tracing_pipeline_trace_rays_indirect = false,
                    .ray_traversal_primitive_culling = true,
                }
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
                .image_usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst,
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
                        .swapchain_index = 0,
                        .framebuffer_render_pass_index = 0,
                    }
                },
            }
        };

        SDL_instance sdl{ SDL_INIT_VIDEO };

        std::pmr::vector<SDL_window> const windows = create_windows(sdl, window_configurations);

        std::pmr::vector<char const*> const required_instance_extensions = [&]
        {
            std::pmr::vector<char const*> instance_extensions = Mythology::SDL::Vulkan::get_sdl_required_instance_extensions({});
            instance_extensions.insert(instance_extensions.end(), instance_configuration.enabled_extensions.begin(), instance_configuration.enabled_extensions.end());
            return instance_extensions;
        }();

        vk::DynamicLoader dynamic_loader;
        PFN_vkGetInstanceProcAddr const get_instance_proccess_address = Mythology::SDL::Vulkan::get_instance_process_address();
        VULKAN_HPP_DEFAULT_DISPATCHER.init(get_instance_proccess_address);

        Mythology::Render::Instance_resources instance_resources
        {
            VK_MAKE_VERSION(instance_configuration.vulkan_version.major, instance_configuration.vulkan_version.minor, instance_configuration.vulkan_version.patch),
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

        check_ray_tracing_support(physical_devices);

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
            render_pipeline_json.contains("descriptor_set_layouts") ? render_pipeline_json.at("descriptor_set_layouts") : nlohmann::json{},
            render_pipeline_json.contains("frame_resources") ? render_pipeline_json.at("frame_resources") : nlohmann::json{},
            render_pipeline_configuration.inputs,
            swapchain_devices,
            swapchain_resources.images,
            swapchain_formats,
            swapchain_image_subresource_ranges,
            swapchain_render_areas,
            render_pipeline_device,
            render_pipeline_resources.render_passes,
            render_pipeline_resources.descriptor_set_layouts,
            number_of_frames_in_flight,
            nullptr,
            {},
            {}
        };

        std::pmr::vector<std::pmr::vector<std::byte>> const data_arrays = Maia::Renderer::Vulkan::create_data_arrays(
            render_pipeline_json.contains("data_arrays") ? render_pipeline_json.at("data_arrays") : nlohmann::json{},
            {}
        );

        nlohmann::json const& command_lists_json = render_pipeline_json.at("frame_commands");
        nlohmann::json const& command_list_json = command_lists_json[render_pipeline_configuration.command_list_index];

        Maia::Renderer::Vulkan::Commands_data const commands_data =
            Maia::Renderer::Vulkan::create_commands_data(
                command_list_json,
                render_pipeline_resources.descriptor_sets,
                render_pipeline_resources.pipeline_states,
                render_pipeline_resources.pipeline_layouts,
                render_pipeline_resources.render_passes,
                render_pipeline_resources.buffer_memory_views,
                Maia::Renderer::Vulkan::get_images(render_pipeline_resources.image_memory_views, {}),
                render_pipeline_resources.shader_binding_tables,
                {},
                {}
        );

        std::optional<Scene_resources> const scene_resources = load_scene(
            m_gltf_path,
            render_pipeline_device,
            render_pipeline_upload_queue,
            render_pipeline_command_pool,
            render_pipeline_upload_buffer.get(),
            buffer_resources,
            nullptr,
            {},
            {}
        );

        if (!scene_resources)
        {
            return {};
        }

        std::pmr::vector<vk::AccelerationStructureKHR> const scene_top_level_acceleration_structures = get_acceleration_structure_handles(
            scene_resources->top_level_acceleration_structures,
            {}
        );

        Frame_shared_resources const frame_shared_resources
        {
            render_pipeline_device,
            (render_pipeline_json.contains("frame_resources") && render_pipeline_json.at("frame_resources").contains("descriptor_sets")) ? render_pipeline_json.at("frame_resources").at("descriptor_sets").at("shared") : nlohmann::json{},
            render_pipeline_json.contains("descriptor_set_layouts") ? render_pipeline_json.at("descriptor_set_layouts") : nlohmann::json{},
            render_pipeline_resources.descriptor_set_layouts,
            scene_top_level_acceleration_structures,
            nullptr,
            {},
            {}
        };

        Maia::Renderer::Vulkan::Frame_descriptor_sets_map const frame_descriptor_sets_map = Maia::Renderer::Vulkan::create_frame_descriptor_sets_map(
            (render_pipeline_json.contains("frame_resources") && render_pipeline_json.at("frame_resources").contains("descriptor_sets")) ? render_pipeline_json.at("frame_resources").at("descriptor_sets").at("descriptor_sets") : nlohmann::json{},
            {}
        );

        std::size_t const swapchain_index = render_pipeline_configuration.inputs[0].swapchain_index;
        std::function<void()> recreate_swapchain = {}; // TODO

        std::pmr::vector<Game_controller> game_controllers;
        game_controllers.reserve(2);

        std::uint8_t frame_index = 0;

        auto previous_time_point = std::chrono::high_resolution_clock::now();
        std::chrono::milliseconds const fixed_time_step = std::chrono::milliseconds{ 20 };
        std::chrono::high_resolution_clock::duration lag = {};

        std::pmr::vector<std::pair<Addon, Addon_interface_pointer>> const addons = load_addons(
            m_addon_paths,
            {}
        );

        std::pmr::vector<Addon_interface*> const addon_interfaces = get_addon_interfaces(
            addons,
            {}
        );

        while (true)
        {
            auto const current_time_point = std::chrono::high_resolution_clock::now();
            auto const elapsed_duration = current_time_point - previous_time_point;
            previous_time_point = current_time_point;
            lag += elapsed_duration;

            process_window_events(
                recreate_swapchain,
                game_controllers,
                Mythology::SDL::add_game_controller,
                Mythology::SDL::remove_game_controller
            );

            process_input(
                addon_interfaces,
                game_controllers,
                {}
            );

            while (lag >= fixed_time_step)
            {
                fixed_update(addon_interfaces);
                lag -= fixed_time_step;
            }

            if (is_fence_signaled(synchronization_resources, frame_index, swapchain_index))
            {
                std::optional<std::uint32_t> const next_swapchain_image_index = acquire_next_image(synchronization_resources, frame_index, swapchain_resources, swapchain_index);

                if (next_swapchain_image_index.has_value())
                {
                    render_frame(
                        render_pipeline_device,
                        render_pipeline_configuration.inputs,
                        pipeline_input_resources,
                        queues,
                        frame_shared_resources,
                        frame_index,
                        frames_command_buffers[frame_index],
                        frame_descriptor_sets_map,
                        synchronization_resources,
                        swapchain_configurations,
                        swapchain_resources,
                        swapchain_render_areas,
                        swapchain_image_subresource_ranges,
                        swapchain_index,
                        *next_swapchain_image_index,
                        data_arrays,
                        commands_data
                    );
                }
                else
                {
                    // TODO may need to recreate swapchain
                }
            }
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
