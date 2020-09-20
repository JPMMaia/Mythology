export module mythology.core.vulkan;

import maia.renderer.vulkan;

import <vulkan/vulkan.h>;

import <cstddef>;
import <fstream>;
import <functional>;
import <memory_resource>;
import <optional>;
import <span>;
import <vector>;

using namespace Maia::Renderer::Vulkan;

namespace Mythology::Core::Vulkan
{
    export VkInstance create_instance(
        std::optional<Application_description> application_description,
        std::optional<Engine_description> engine_description,
        API_version api_version,
        std::span<char const* const> required_extensions = {}) noexcept;
    
    export Physical_device select_physical_device(VkInstance instance) noexcept;

    export Queue_family_index find_graphics_queue_family_index(
        Physical_device physical_device
    ) noexcept;

    export Queue_family_index find_present_queue_family_index(
        Physical_device physical_device,
        Surface surface,
        std::optional<Queue_family_index> preference = {}
    ) noexcept;
    
    export Device create_device(
        Physical_device physical_device,
        std::span<Queue_family_index const> queue_family_indices,
        std::function<bool(VkExtensionProperties)> const& is_extension_to_enable) noexcept;

    export struct Device_memory_and_color_image
    {
        VkDeviceMemory device_memory;
        Image color_image;
    };

    export Device_memory_and_color_image create_device_memory_and_color_image(
        Physical_device physical_device,
        Device device,
        VkFormat format,
        VkExtent3D extent
    ) noexcept;

    export Render_pass create_render_pass(
        Device device,
        VkFormat color_image_format) noexcept;

    export VkPipeline create_vertex_and_fragment_pipeline(
        Device device,
        std::optional<VkPipelineCache> pipeline_cache,
        VkPipelineLayout pipeline_layout,
        VkRenderPass render_pass,
        std::uint32_t subpass_index,
        std::uint32_t subpass_attachment_count,
        VkShaderModule vertex_shader,
        VkShaderModule fragment_shader
    ) noexcept;

    export void clear_and_begin_render_pass(
        Command_buffer command_buffer,
        Render_pass render_pass,
        Framebuffer framebuffer,
        VkClearColorValue clear_color,
        Image output_image,
        VkImageSubresourceRange output_image_subresource_range,
        VkRect2D output_render_area
    ) noexcept;

    export void end_render_pass_and_switch_layout(
        Command_buffer command_buffer,
        Image output_image,
        VkImageSubresourceRange output_image_subresource_range,
        bool switch_to_present_layout
    ) noexcept;

    export std::pmr::vector<std::byte> read_memory(
        Device device,
        VkDeviceMemory device_memory,
        VkSubresourceLayout subresource_layout,
        std::pmr::polymorphic_allocator<std::byte> const& allocator = {}
    ) noexcept;

    export void write_p3(
        std::ostream& output_stream,
        std::span<std::byte const> data_to_write,
        VkSubresourceLayout subresource_layout,
        VkExtent3D subresource_extent
    ) noexcept;
}