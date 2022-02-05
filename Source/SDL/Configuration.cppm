module;

#include <nlohmann/json.hpp>
#include <SDL2/SDL.h>
#include <vulkan/vulkan.hpp>

#include <cstdint>
#include <memory_resource>
#include <span>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

export module mythology.sdl.configuration;

import maia.renderer.vulkan.serializer;
import mythology.sdl.render_resources;
import mythology.sdl.sdl;

namespace Mythology::SDL
{
    export struct Vulkan_version
    {
        std::uint32_t major = {};
        std::uint32_t minor = {};
        std::uint32_t patch = {};
    };

    export struct Instance_configuration
    {
        Vulkan_version vulkan_version = {};
        std::pmr::vector<char const*> enabled_extensions;
    };

    export struct Window_offset
    {
        int x = 0;
        int y = 0;
    };

    export struct Window_extent
    {
        int width = 0;
        int height = 0;
    };

    export struct Fullscreen_mode
    {
        unsigned int display_index;
    };

    export struct Windowed_mode
    {
        Window_offset offset;
        Window_extent extent;
    };

    export struct Window_configuration
    {
        std::pmr::string title;
        std::variant<Fullscreen_mode, Windowed_mode> mode;
    };

    export std::pmr::vector<Mythology::SDL::SDL_window> create_windows(
        Mythology::SDL::SDL_instance const& sdl,
        std::span<Window_configuration const> const window_configurations
    );

    export struct Surface_configuration
    {
        std::uint8_t window_index = 0;
    };

    export std::pmr::vector<SDL_Window*> select_surface_windows(
        std::span<Surface_configuration const> const surface_configurations,
        std::span<Mythology::SDL::SDL_window const> windows,
        std::pmr::polymorphic_allocator<> const& allocator
    );

    export struct Physical_device_configuration
    {
        std::uint32_t vendor_ID = 0;
        std::uint32_t device_ID = 0;
    };

    export std::pmr::vector<vk::PhysicalDevice> get_physical_devices(
        std::span<Physical_device_configuration const> const configurations,
        vk::Instance const instance,
        std::pmr::polymorphic_allocator<> const& allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    );

    export struct Queue_create_info_configuration
    {
        std::uint32_t queue_family_index = 0;
        std::pmr::vector<float> priorities;

        std::uint32_t count() const noexcept;
    };

    export struct Physical_device_vulkan_12_features
    {
        bool sampler_mirror_clamp_to_edge = {};
        bool draw_indirect_count = {};
        bool storage_buffer8_bit_access = {};
        bool uniform_and_storage_buffer8_bit_access = {};
        bool storage_push_constant8 = {};
        bool shader_buffer_int64_atomics = {};
        bool shader_shared_int64_atomics = {};
        bool shader_float16 = {};
        bool shader_int8 = {};
        bool descriptor_indexing = {};
        bool shader_input_attachment_array_dynamic_indexing = {};
        bool shader_uniform_texel_buffer_array_dynamic_indexing = {};
        bool shader_storage_texel_buffer_array_dynamic_indexing = {};
        bool shader_uniform_buffer_array_non_uniform_indexing = {};
        bool shader_sampled_image_array_non_uniform_indexing = {};
        bool shader_storage_buffer_array_non_uniform_indexing = {};
        bool shader_storage_image_array_non_uniform_indexing = {};
        bool shader_input_attachment_array_non_uniform_indexing = {};
        bool shader_uniform_texel_buffer_array_non_uniform_indexing = {};
        bool shader_storage_texel_buffer_array_non_uniform_indexing = {};
        bool descriptor_binding_uniform_buffer_update_after_bind = {};
        bool descriptor_binding_sampled_image_update_after_bind = {};
        bool descriptor_binding_storage_image_update_after_bind = {};
        bool descriptor_binding_storage_buffer_update_after_bind = {};
        bool descriptor_binding_uniform_texel_buffer_update_after_bind = {};
        bool descriptor_binding_storage_texel_buffer_update_after_bind = {};
        bool descriptor_binding_update_unused_while_pending = {};
        bool descriptor_binding_partially_bound = {};
        bool descriptor_binding_variable_descriptor_count = {};
        bool runtime_descriptor_array = {};
        bool sampler_filter_minmax = {};
        bool scalar_block_layout = {};
        bool imageless_framebuffer = {};
        bool uniform_buffer_standard_layout = {};
        bool shader_subgroup_extended_types = {};
        bool separate_depth_stencil_layouts = {};
        bool host_query_reset = {};
        bool timeline_semaphore = {};
        bool buffer_device_address = {};
        bool buffer_device_address_capture_replay = {};
        bool buffer_device_address_multi_device = {};
        bool vulkan_memory_model = {};
        bool vulkan_memory_model_device_scope = {};
        bool vulkan_memory_model_availability_visibility_chains = {};
        bool shader_output_viewport_index = {};
        bool shader_output_layer = {};
        bool subgroup_broadcast_dynamic_id = {};
    };

    export struct Physical_device_acceleration_structure_features
    {
        bool acceleration_structure = {};
        bool acceleration_structure_capture_replay = {};
        bool acceleration_structure_indirect_build = {};
        bool acceleration_structure_host_commands = {};
        bool descriptor_binding_acceleration_structure_update_after_bind = {};
    };

    export struct Ray_tracing_features_configuration
    {
        bool ray_tracing_pipeline = {};
        bool ray_tracing_pipeline_shader_group_handle_capture_replay = {};
        bool ray_tracing_pipeline_shader_group_handle_capture_replay_mixed = {};
        bool ray_tracing_pipeline_trace_rays_indirect = {};
        bool ray_traversal_primitive_culling = {};
    };

    export struct Device_configuration
    {
        std::uint32_t physical_device_index = 0;
        std::pmr::vector<Queue_create_info_configuration> queues;
        std::pmr::vector<char const*> enabled_extensions;
        std::uint32_t upload_queue_index = 0;
        Physical_device_vulkan_12_features vulkan_12_features = {};
        Physical_device_acceleration_structure_features acceleration_structure_features = {};
        Ray_tracing_features_configuration ray_tracing_features = {};
    };

    export struct Device_resources
    {
        Device_resources(
            std::span<Device_configuration const> configurations,
            std::span<vk::PhysicalDevice const> physical_devices,
            std::pmr::polymorphic_allocator<> const& allocator,
            std::pmr::polymorphic_allocator<> const& temporaries_allocator
        );
        Device_resources(Device_resources const&) = delete;
        Device_resources(Device_resources&& other) noexcept;
        ~Device_resources() noexcept;

        Device_resources& operator=(Device_resources const&) = delete;
        Device_resources& operator=(Device_resources&& other) noexcept;

        std::pmr::vector<vk::Device> devices;
    };

    export struct Queue_configuration
    {
        std::uint32_t device_index = 0;
        std::uint32_t queue_family_index = 0;
        std::uint32_t queue_index = 0;
    };

    export std::pmr::vector<vk::Queue> get_queues(
        std::span<Queue_configuration const> const configurations,
        std::span<vk::Device const> const devices,
        std::pmr::polymorphic_allocator<> const& allocator
    );

    export struct Swapchain_configuration
    {
        std::uint32_t device_index = 0;
        std::uint32_t surface_index = 0;
        vk::SwapchainCreateFlagsKHR flags = {};
        std::uint32_t minimum_image_count = 3;
        vk::Format image_format = vk::Format::eUndefined;
        vk::ColorSpaceKHR image_color_space = vk::ColorSpaceKHR::eSrgbNonlinear;
        std::uint32_t image_array_layers = 1;
        vk::ImageUsageFlags image_usage = vk::ImageUsageFlagBits::eColorAttachment;
        vk::SharingMode image_sharing_mode = vk::SharingMode::eExclusive;
        std::pmr::vector<std::uint32_t> queue_family_indices = {};
        vk::CompositeAlphaFlagBitsKHR composite_alpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        vk::PresentModeKHR present_mode = vk::PresentModeKHR::eFifo;
        bool clipped = true;
        std::uint32_t queue_to_present_index = 0;

        std::uint32_t queue_family_index_count() const noexcept;
    };

    export std::pmr::vector<vk::Device> get_swapchain_devices(
        std::span<Swapchain_configuration const> configurations,
        std::span<vk::Device const> devices,
        std::pmr::polymorphic_allocator<> const& allocator
    );

    export std::pmr::vector<vk::Extent2D> get_image_extents(
        std::span<Surface_configuration const> configurations,
        std::span<SDL_Window* const> windows,
        std::pmr::polymorphic_allocator<> const& allocator
    );

    export bool validate_swapchain_queues(
        std::span<Device_configuration const> device_configurations,
        std::span<Queue_configuration const> queue_configurations,
        std::span<Swapchain_configuration const> swapchain_configurations,
        std::span<vk::PhysicalDevice const> physical_devices,
        std::span<vk::SurfaceKHR const> surfaces
    ) noexcept;

    export std::pmr::vector<vk::SurfaceCapabilitiesKHR> get_swapchain_surface_capabilities(
        std::span<Swapchain_configuration const> swapchain_configurations,
        std::span<Device_configuration const> device_configurations,
        std::span<vk::PhysicalDevice const> physical_devices,
        std::span<vk::SurfaceKHR const> surfaces,
        std::pmr::polymorphic_allocator<> const& allocator
    );

    export struct Swapchain_resources
    {
        Swapchain_resources(
            std::span<Swapchain_configuration const> configurations,
            std::span<vk::Device const> devices,
            std::span<vk::SurfaceKHR const> surfaces,
            std::span<vk::Extent2D const> image_extents,
            std::span<vk::SurfaceCapabilitiesKHR const> swapchain_surface_capabilities,
            std::pmr::polymorphic_allocator<> const& allocator
        );
        Swapchain_resources(Swapchain_resources const&) = delete;
        Swapchain_resources(Swapchain_resources&& other) noexcept;
        ~Swapchain_resources() noexcept;

        Swapchain_resources& operator=(Swapchain_resources const&) = delete;
        Swapchain_resources& operator=(Swapchain_resources&& other) noexcept;

        std::pmr::vector<vk::Device> devices;
        std::pmr::vector<vk::SwapchainKHR> swapchains;
        std::pmr::vector<std::pmr::vector<vk::Image>> images;
    };

    export std::pmr::vector<vk::Format> get_swapchain_image_formats(
        std::span<Swapchain_configuration const> configurations,
        std::pmr::polymorphic_allocator<> const& allocator
    );

    export std::pmr::vector<vk::Device> get_queue_devices(
        std::span<Queue_configuration const> configurations,
        std::span<vk::Device const> devices,
        std::pmr::polymorphic_allocator<> const& allocator
    );

    export std::pmr::vector<std::uint32_t> get_queue_family_indices(
        std::span<Queue_configuration const> configurations,
        std::pmr::polymorphic_allocator<> const& allocator
    );

    export struct Render_pipeline_input_configuration
    {
        std::uint32_t swapchain_index = 0;
        std::uint32_t framebuffer_render_pass_index = 0;
    };

    export struct Render_pipeline_configuration
    {
        std::pmr::string name;
        std::uint32_t command_list_index = 0;
        std::uint32_t device_index = 0;
        std::pmr::vector<Render_pipeline_input_configuration> inputs;
    };

    export std::pmr::vector<vk::Image> get_input_images(
        std::span<Render_pipeline_input_configuration const> inputs,
        std::span<std::pmr::vector<vk::Image> const> swapchain_images,
        std::span<std::uint32_t const> swapchain_image_indices,
        std::pmr::polymorphic_allocator<> const& allocator
    );

    export std::pmr::vector<vk::ImageView> get_input_image_views(
        std::span<Render_pipeline_input_configuration const> inputs,
        std::span<std::pmr::vector<vk::ImageView> const> swapchain_image_views,
        std::span<std::uint32_t const> swapchain_image_indices,
        std::pmr::polymorphic_allocator<> const& allocator
    );

    export std::pmr::vector<vk::Framebuffer> get_input_framebuffers(
        std::span<Render_pipeline_input_configuration const> inputs,
        std::span<std::pmr::vector<vk::Framebuffer> const> swapchain_framebuffers,
        std::span<std::uint32_t const> swapchain_image_indices,
        std::pmr::polymorphic_allocator<> const& allocator
    );

    export std::pmr::vector<vk::ImageSubresourceRange> get_image_subresource_ranges(
        std::span<Render_pipeline_input_configuration const> inputs,
        std::pmr::polymorphic_allocator<> const& allocator
    );

    export std::pmr::vector<vk::Rect2D> get_render_areas(
        std::span<Render_pipeline_input_configuration const> inputs,
        std::span<Swapchain_configuration const> swapchain_configurations,
        std::span<vk::Extent2D const> surface_image_extents,
        std::pmr::polymorphic_allocator<> const& allocator
    );

    export struct Render_pipeline_input_resources
    {
        Render_pipeline_input_resources(
            nlohmann::json const& descriptor_set_layouts_json,
            nlohmann::json const& frame_resources_json,
            std::span<Render_pipeline_input_configuration const> const inputs,
            std::span<vk::Device const> const swapchain_devices,
            std::span<std::pmr::vector<vk::Image> const> swapchain_images,
            std::span<vk::Format const> swapchain_image_formats,
            std::span<vk::ImageSubresourceRange const> swapchain_image_subresource_ranges,
            std::span<vk::Rect2D const> swapchain_render_areas,
            vk::Device render_pipeline_device,
            std::span<vk::RenderPass const> render_pipeline_render_passes,
            std::span<vk::DescriptorSetLayout const> render_pipeline_descriptor_set_layouts,
            std::uint32_t frames_in_flight,
            vk::AllocationCallbacks const* allocation_callbacks,
            std::pmr::polymorphic_allocator<> const& output_allocator,
            std::pmr::polymorphic_allocator<> const& temporaries_allocator
        );
        
        Render_pipeline_input_resources(Render_pipeline_input_resources const&) = delete;
        Render_pipeline_input_resources(Render_pipeline_input_resources&& other) noexcept = default;
        ~Render_pipeline_input_resources() noexcept;

        Render_pipeline_input_resources& operator=(Render_pipeline_input_resources const&) = delete;
        Render_pipeline_input_resources& operator=(Render_pipeline_input_resources&&) noexcept = default;

        std::pmr::vector<vk::Device> swapchain_devices;
        std::pmr::vector<std::pmr::vector<vk::ImageView>> image_views;
        std::pmr::vector<std::pmr::vector<vk::Framebuffer>> framebuffers;
        vk::DescriptorPool descriptor_pool;
        std::pmr::vector<std::pmr::vector<vk::DescriptorSet>> descriptor_sets;
        std::pmr::vector<std::pmr::vector<Maia::Renderer::Vulkan::Frame_descriptor_set_binding>> descriptor_sets_bindings;
        std::pmr::vector<std::pmr::vector<std::pmr::vector<vk::ImageLayout>>> descriptor_sets_image_layouts;
        std::pmr::vector<std::pmr::vector<std::pmr::vector<std::size_t>>> descriptor_sets_image_indices;
    };
}
