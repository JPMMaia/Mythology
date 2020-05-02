module mythology.windowless;

import mythology.core.utilities;
import mythology.core.vulkan;
import maia.renderer.vulkan;

import <vulkan/vulkan.h>;

import <cassert>;
import <cstring>;
import <filesystem>;
import <fstream>;
import <functional>;
import <memory_resource>;
import <optional>;
import <span>;
import <vector>;

using namespace Maia::Renderer::Vulkan;
using namespace Mythology::Core;
using namespace Mythology::Core::Vulkan;

namespace Mythology::Windowless
{
    namespace
    {
        struct Device_resources
        {
            explicit Device_resources(API_version const api_version) noexcept
            {
                using namespace Mythology::Core::Vulkan;

                this->instance = create_instance(Application_description{"Mythology", 1}, Engine_description{"Mythology Engine", 1}, api_version);

                this->physical_device = select_physical_device(this->instance);
                
                this->graphics_queue_family_index = find_graphics_queue_family_index(this->physical_device);

                auto const is_extension_to_enable = [](VkExtensionProperties const& properties) -> bool
                {
                    return false;
                };

                this->device = create_device(this->physical_device, {&this->graphics_queue_family_index, 1}, is_extension_to_enable);

                this->fence = create_fence(this->device, {}, {});
        
                this->command_pool = create_command_pool(this->device, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, this->graphics_queue_family_index, {});
            }
            Device_resources(Device_resources const&) = delete;
            Device_resources(Device_resources&&) = delete;
            ~Device_resources() noexcept
            {
                if (this->command_pool.value != VK_NULL_HANDLE)
                {
                    destroy_command_pool(this->device, this->command_pool, {});
                }

                if (this->fence.value != VK_NULL_HANDLE)
                {
                    destroy_fence(this->device, this->fence, {});
                }

                if (this->device.value != VK_NULL_HANDLE)
                {
                    destroy_device(this->device);
                }

                if (this->instance.value != VK_NULL_HANDLE)
                {
                    destroy_instance(this->instance);
                }
            }

            Device_resources& operator=(Device_resources const&) = delete;
            Device_resources& operator=(Device_resources&&) = delete;

            Instance instance = {};
            Physical_device physical_device = {};
            Queue_family_index graphics_queue_family_index = {};
            Device device = {};
            Fence fence = {};
            Command_pool command_pool = {};
        };

        struct Application_resources
        {
            explicit Application_resources(
                std::filesystem::path const shaders_path,
                Physical_device const physical_device,
                Device const device,
                VkFormat const color_image_format,
                VkExtent3D const color_image_extent) noexcept :
                device{device},
                device_memory_and_color_image(Mythology::Core::Vulkan::create_device_memory_and_color_image(physical_device, device, color_image_format, color_image_extent)),
                pipeline_layout{create_pipeline_layout(device.value, empty_pipeline_layout_create_info(), {})},
                render_pass{create_render_pass(device, color_image_format)},
                triangle_vertex_shader_module{create_shader_module(device, {}, convert_bytes<std::uint32_t>(read_bytes(shaders_path / "Triangle.vertex.spv")))},
                white_fragment_shader_module{create_shader_module(device, {}, convert_bytes<std::uint32_t>(read_bytes(shaders_path / "White.fragment.spv")))},
                white_triangle_pipeline{create_vertex_and_fragment_pipeline(device, {}, pipeline_layout, render_pass.value, 0, 1, triangle_vertex_shader_module.value, white_fragment_shader_module.value)},
                color_image_view{create_image_view(device, {}, device_memory_and_color_image.color_image, VK_IMAGE_VIEW_TYPE_2D, color_image_format, {}, {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1}, {})},
                framebuffer{create_framebuffer(device, {}, render_pass, {&color_image_view.value, 1}, {color_image_extent.width, color_image_extent.height, 1}, {})}
            {
            }
            Application_resources(Application_resources const&) = delete;
            Application_resources(Application_resources&&) = delete;
            ~Application_resources() noexcept
            {
                if (color_image_view.value != VK_NULL_HANDLE)
                {
                    destroy_image_view(device, color_image_view, {});
                }

                if (framebuffer.value != VK_NULL_HANDLE)
                {
                    destroy_framebuffer(device, framebuffer, {});
                }

                if (white_triangle_pipeline != VK_NULL_HANDLE)
                {
                    vkDestroyPipeline(device.value, white_triangle_pipeline, nullptr);
                }

                if (triangle_vertex_shader_module.value != VK_NULL_HANDLE)
                {
                    destroy_shader_module(device, triangle_vertex_shader_module);
                }

                if (white_fragment_shader_module.value != VK_NULL_HANDLE)
                {
                    destroy_shader_module(device, white_fragment_shader_module);
                }

                if (render_pass.value != VK_NULL_HANDLE)
                {
                    destroy_render_pass(device, render_pass, {});
                }

                if (pipeline_layout != VK_NULL_HANDLE)
                {
                    destroy_pipeline_layout(device.value, pipeline_layout, {});
                }

                if (this->device_memory_and_color_image.color_image.value != VK_NULL_HANDLE)
                {
                    destroy_image(this->device, this->device_memory_and_color_image.color_image, {});
                }

                if (this->device_memory_and_color_image.device_memory != VK_NULL_HANDLE)
                {
                    free_memory(this->device.value, this->device_memory_and_color_image.device_memory, {});
                }
            }

            Application_resources& operator=(Application_resources const&) = delete;
            Application_resources& operator=(Application_resources&&) = delete;

            Device device;
            Mythology::Core::Vulkan::Device_memory_and_color_image device_memory_and_color_image;
            VkPipelineLayout pipeline_layout;
            Render_pass render_pass;
            Shader_module triangle_vertex_shader_module;
            Shader_module white_fragment_shader_module;
            VkPipeline white_triangle_pipeline;
            Image_view color_image_view;
            Framebuffer framebuffer;
        };
    }

    void render_frame(
        std::filesystem::path const& output_filename
    ) noexcept
    {
        using namespace Mythology::Core::Vulkan;
        using namespace Maia::Renderer::Vulkan;
        
        std::filesystem::path const shaders_path = std::filesystem::current_path() / "../shaders";

        Device_resources const device_resources{make_api_version(1, 2, 0)};
        Physical_device const physical_device = device_resources.physical_device;
        Device const device = device_resources.device;
        Queue_family_index const graphics_queue_family_index = device_resources.graphics_queue_family_index;
        Fence const fence = device_resources.fence;
        Command_pool const command_pool = device_resources.command_pool;
        
        Queue const queue = get_device_queue(device, graphics_queue_family_index, 0);
        
        VkFormat constexpr color_image_format = VK_FORMAT_R8G8B8A8_UINT;
        VkExtent3D constexpr color_image_extent{16, 16, 1};
        Application_resources const application_resources{shaders_path, physical_device, device, color_image_format, color_image_extent};
        VkDeviceMemory const color_image_device_memory = application_resources.device_memory_and_color_image.device_memory;
        Image const color_image = application_resources.device_memory_and_color_image.color_image;
        Render_pass const render_pass = application_resources.render_pass;
        Framebuffer const framebuffer = application_resources.framebuffer;
        VkPipeline const white_triangle_pipeline = application_resources.white_triangle_pipeline;

        {
            std::pmr::vector<Command_buffer> const command_buffers = 
                allocate_command_buffers(
                    device,
                    command_pool,
                    VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                    1,
                    {}
                );
            assert(command_buffers.size() == 1);
            Command_buffer const command_buffer = command_buffers.front();

            begin_command_buffer(command_buffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, {});
            {
                VkClearColorValue const clear_color =
                {
                    .uint32 = {0, 128, 0, 255}
                };

                VkRect2D const output_render_area
                {
                    .offset = {0, 0},
                    .extent = {color_image_extent.width, color_image_extent.height}
                };

                render(command_buffer, render_pass, framebuffer, clear_color, white_triangle_pipeline, color_image, output_render_area);
            }
            end_command_buffer(command_buffer);

            queue_submit(queue, {}, {}, {&command_buffer, 1}, {}, fence);
        }
        check_result(
            wait_for_all_fences(device, {&fence, 1}, Timeout_nanoseconds{100000}));

        {
            VkSubresourceLayout const color_image_layout = get_subresource_layout(
                device,
                color_image,
                {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .arrayLayer = 0}
            );

            std::pmr::vector<std::byte> const color_image_data = 
                read_memory(device, color_image_device_memory, color_image_layout);

            std::ofstream output_file{output_filename};
            write_p3(output_file, color_image_data, color_image_layout, color_image_extent);
        }        
    }
}