module mythology.windowless;

import mythology.core.utilities;
import mythology.core.vulkan;
import maia.renderer.vulkan;
import maia.renderer.vulkan.serializer;

import <nlohmann/json.hpp>;
import <vulkan/vulkan.h>;

import <cassert>;
import <cstring>;
import <exception>;
import <filesystem>;
import <fstream>;
import <functional>;
import <iostream>;
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
        VkBool32 terminate_if_error(
            VkDebugUtilsMessageSeverityFlagBitsEXT const message_severity,
            VkDebugUtilsMessageTypeFlagsEXT const message_types,
            VkDebugUtilsMessengerCallbackDataEXT const* callback_data,
            void* user_data
        ) noexcept
        {
            if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            {
                std::cerr << callback_data->pMessage << std::endl;
                std::terminate();
            }
                
            return VK_FALSE;
        }

        VkDebugUtilsMessengerEXT create_debug_messenger(
            VkInstance const instance,
            PFN_vkDebugUtilsMessengerCallbackEXT debug_callback,
            VkAllocationCallbacks const* vulkan_allocator
        ) noexcept
        {
            VkDebugUtilsMessengerCreateInfoEXT const create_info
            {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                .messageSeverity = 
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                .messageType = 
                    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                .pfnUserCallback = debug_callback,
                .pUserData = nullptr,
            };
            
            VkDebugUtilsMessengerEXT debug_messenger = {};

            PFN_vkCreateDebugUtilsMessengerEXT create_debug_utils_messenger =
                reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
                    vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")
                );

            check_result(
                create_debug_utils_messenger(
                    instance,
                    &create_info,
                    vulkan_allocator,
                    &debug_messenger
                )
            );

            return debug_messenger;
        }

        struct Device_resources
        {
            explicit Device_resources(API_version const api_version) noexcept
            {
                using namespace Mythology::Core::Vulkan;

                this->instance = create_instance(Application_description{"Mythology", 1}, Engine_description{"Mythology Engine", 1}, api_version);

                this->debug_messenger = create_debug_messenger(this->instance.value, terminate_if_error, nullptr);

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

                if (this->debug_messenger != VK_NULL_HANDLE)
                {
                    PFN_vkDestroyDebugUtilsMessengerEXT destroy_debug_utils_messenger =
                        reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                            vkGetInstanceProcAddr(this->instance.value, "vkDestroyDebugUtilsMessengerEXT")
                        );

                    destroy_debug_utils_messenger(
                        this->instance.value,
                        this->debug_messenger,
                        nullptr
                    );
                }

                if (this->instance.value != VK_NULL_HANDLE)
                {
                    destroy_instance(this->instance);
                }
            }

            Device_resources& operator=(Device_resources const&) = delete;
            Device_resources& operator=(Device_resources&&) = delete;

            Instance instance = {};
            VkDebugUtilsMessengerEXT debug_messenger = {};
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
                VkExtent3D const color_image_extent,
                std::optional<VkRenderPass> const render_pass) noexcept :
                device{device},
                device_memory_and_color_image(Mythology::Core::Vulkan::create_device_memory_and_color_image(physical_device, device, color_image_format, color_image_extent)),
                color_image_view{create_image_view(device, {}, device_memory_and_color_image.color_image, VK_IMAGE_VIEW_TYPE_2D, color_image_format, {}, {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1}, {})},
                framebuffer{render_pass.has_value() ? create_framebuffer(device, {}, {*render_pass}, {&color_image_view.value, 1}, {color_image_extent.width, color_image_extent.height, 1}, {}).value : std::optional<VkFramebuffer>{}}
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

                if (framebuffer.has_value() && *framebuffer != VK_NULL_HANDLE)
                {
                    destroy_framebuffer(device, {*framebuffer}, {});
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
            Image_view color_image_view;
            std::optional<VkFramebuffer> framebuffer;
        };
    }

    std::pmr::vector<std::byte> render_frame(
        Frame_dimensions const frame_dimensions,
        nlohmann::json const& pipeline_json,
        std::filesystem::path const& pipeline_json_parent_path,
        std::optional<std::filesystem::path> const& gltf_file_path,
        std::pmr::polymorphic_allocator<std::byte> const& output_allocator,
        std::pmr::polymorphic_allocator<std::byte> const& temporaries_allocator
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
        
        std::pmr::vector<VkRenderPass> const render_passes = 
            Maia::Renderer::Vulkan::create_render_passes(device.value, nullptr, pipeline_json.at("render_passes"), output_allocator, temporaries_allocator);

        VkFormat constexpr color_image_format = VK_FORMAT_R8G8B8A8_UINT;
        VkExtent3D const color_image_extent{frame_dimensions.width, frame_dimensions.height, 1};
        std::optional<VkRenderPass> const framebuffer_render_pass = !render_passes.empty() ? render_passes[0] : std::optional<VkRenderPass>{};
        Application_resources const application_resources{shaders_path, physical_device, device, color_image_format, color_image_extent, framebuffer_render_pass};
        VkDeviceMemory const color_image_device_memory = application_resources.device_memory_and_color_image.device_memory;
        VkImage const color_image = application_resources.device_memory_and_color_image.color_image.value;
        std::optional<VkFramebuffer> const framebuffer = application_resources.framebuffer;

        std::pmr::vector<VkShaderModule> const shader_modules = 
            Maia::Renderer::Vulkan::create_shader_modules(device.value, nullptr, pipeline_json.at("shader_modules"), pipeline_json_parent_path, output_allocator, temporaries_allocator);

        std::pmr::vector<VkSampler> const samplers = 
            Maia::Renderer::Vulkan::create_samplers(device.value, nullptr, pipeline_json.at("samplers"), output_allocator);
        
        std::pmr::vector<VkDescriptorSetLayout> const descriptor_set_layouts = 
            Maia::Renderer::Vulkan::create_descriptor_set_layouts(device.value, nullptr, samplers, pipeline_json.at("descriptor_set_layouts"), output_allocator, temporaries_allocator);

        std::pmr::vector<VkPipelineLayout> const pipeline_layouts = 
            Maia::Renderer::Vulkan::create_pipeline_layouts(device.value, nullptr, descriptor_set_layouts, pipeline_json.at("pipeline_layouts"), output_allocator, temporaries_allocator);

        std::pmr::vector<VkPipeline> const pipeline_states = create_pipeline_states(
            device.value,
            nullptr,
            shader_modules,
            pipeline_layouts,
            render_passes,
            pipeline_json.at("pipeline_states"),
            output_allocator,
            temporaries_allocator
        );

        Maia::Renderer::Vulkan::Commands_data const commands_data = Maia::Renderer::Vulkan::create_commands_data(
            pipeline_json.at("frame_commands"),
            pipeline_states,
            render_passes,
            output_allocator,
            temporaries_allocator
        );

        {
            std::pmr::vector<Command_buffer> const command_buffers = 
                allocate_command_buffers(
                    device,
                    command_pool,
                    VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                    1,
                    {},
                    output_allocator
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

                VkImageSubresourceRange const output_image_subresource_range
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, 
                    .baseMipLevel = 0,
                    .levelCount = 1, 
                    .baseArrayLayer = 0,
                    .layerCount = 1
                };

                Maia::Renderer::Vulkan::draw(
                    command_buffer.value,
                    color_image,
                    output_image_subresource_range,
                    framebuffer,
                    output_render_area,
                    commands_data,
                    temporaries_allocator
                );
            }
            end_command_buffer(command_buffer);

            queue_submit(queue, {}, {}, {&command_buffer, 1}, {}, fence);
        }
        check_result(
            wait_for_all_fences(device, {&fence, 1}, Timeout_nanoseconds{100000}));

        VkSubresourceLayout const color_image_layout = get_subresource_layout(
            device,
            {color_image},
            {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .arrayLayer = 0}
        );

        std::pmr::vector<std::byte> const color_image_data = 
            read_memory(device, color_image_device_memory, color_image_layout, output_allocator);

        return color_image_data;       
    }
}