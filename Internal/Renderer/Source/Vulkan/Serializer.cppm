module;

#include <nlohmann/json.hpp>
#include <vulkan/vulkan.hpp>

#include <cstddef>
#include <filesystem>
#include <memory_resource>
#include <optional>
#include <span>
#include <vector>


export module maia.renderer.vulkan.serializer;

import maia.renderer.vulkan.buffer_resources;
import maia.renderer.vulkan.image_resources;
import maia.renderer.vulkan.upload;

namespace Maia::Renderer::Vulkan
{
    export vk::DescriptorPool create_descriptor_pool(
        nlohmann::json const& descriptor_sets_json,
        nlohmann::json const& descriptor_set_layouts_json,
        vk::Device device,
        std::uint32_t descriptor_set_count_multiplier,
        vk::AllocationCallbacks const* allocation_callbacks,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    );

    export std::pmr::vector<vk::DescriptorSet> create_descriptor_sets(
        nlohmann::json const& descriptor_sets_json,
        vk::Device device,
        vk::DescriptorPool descriptor_pool,
        std::span<vk::DescriptorSetLayout const> descriptor_set_layouts,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    );

    export void update_descriptor_sets(
        nlohmann::json const& descriptor_sets_json,
        vk::Device device,
        std::span<vk::DescriptorSet const> descriptor_sets,
        std::span<Maia::Renderer::Vulkan::Buffer_view const> buffers,
        std::span<vk::BufferView const> buffer_views,
        std::span<vk::ImageView const> image_views,
        std::span<vk::Sampler const> samplers,
        std::span<vk::AccelerationStructureKHR const> acceleration_structures,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    );

    export struct Frame_descriptor_set_binding
    {
        vk::DescriptorType descriptor_type = {};
        std::uint32_t binding = {};
        std::uint32_t first_array_element = {};
    };

    export std::pmr::vector<std::pmr::vector<vk::DescriptorSet>> create_frame_descriptor_sets(
        nlohmann::json const& frame_descriptor_sets_json,
        vk::Device device,
        vk::DescriptorPool descriptor_pool,
        std::span<vk::DescriptorSetLayout const> descriptor_set_layouts,
        std::size_t frames_in_flight,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    );

    export std::pmr::vector<std::pmr::vector<Frame_descriptor_set_binding>> create_descriptor_sets_bindings(
        nlohmann::json const& frame_descriptor_sets,
        std::pmr::polymorphic_allocator<> const& output_allocator
    );

    export std::pmr::vector<std::pmr::vector<std::pmr::vector<vk::ImageLayout>>> create_descriptor_sets_image_layouts(
        nlohmann::json const& frame_descriptor_sets,
        std::pmr::polymorphic_allocator<> const& output_allocator
    );

    export std::pmr::vector<std::pmr::vector<std::pmr::vector<std::size_t>>> create_descriptor_sets_image_indices(
        nlohmann::json const& frame_descriptor_sets,
        std::span<std::size_t const> input_index_to_image_index,
        std::pmr::polymorphic_allocator<> const& output_allocator
    );

    export void update_frame_descriptor_sets(
        vk::Device device,
        std::span<vk::DescriptorSet const> frame_descriptor_sets,
        std::span<std::pmr::vector<std::pmr::vector<std::size_t>> const> descriptor_sets_image_indices,
        std::span<std::pmr::vector<std::pmr::vector<vk::ImageLayout>> const> descriptor_sets_image_layouts,
        std::span<vk::ImageView const> frame_image_views,
        std::span<std::pmr::vector<Frame_descriptor_set_binding> const> descriptor_set_bindings,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    );

    export struct Render_pass_create_info_resources
    {
        std::pmr::vector<vk::AttachmentDescription> attachments;
        std::pmr::vector<vk::AttachmentReference> attachment_references;
        std::pmr::vector<std::uint32_t> preserve_attachments;
        std::pmr::vector<vk::SubpassDescription> subpasses;
        std::pmr::vector<vk::SubpassDependency> dependencies;
        vk::RenderPassCreateInfo create_info;
    };

    export Render_pass_create_info_resources create_render_pass_create_info_resources(
        nlohmann::json const& render_pass_json,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    ) noexcept;

    export std::pmr::vector<vk::RenderPass> create_render_passes(
        vk::Device device,
        vk::AllocationCallbacks const* allocation_callbacks,
        nlohmann::json const& render_passes_json,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    ) noexcept;


    export std::pmr::vector<vk::ShaderModule> create_shader_modules(
        vk::Device const device,
        vk::AllocationCallbacks const* const allocation_callbacks,
        nlohmann::json const& shader_modules_json,
        std::filesystem::path const& shaders_path,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    ) noexcept;


    export std::pmr::vector<vk::Sampler> create_samplers(
        vk::Device device,
        vk::AllocationCallbacks const* allocation_callbacks,
        nlohmann::json const& samplers_json,
        std::pmr::polymorphic_allocator<> const& output_allocator
    ) noexcept;


    export std::pmr::vector<vk::DescriptorSetLayout> create_descriptor_set_layouts(
        vk::Device device,
        vk::AllocationCallbacks const* allocation_callbacks,
        std::span<vk::Sampler const> samplers,
        nlohmann::json const& descriptor_set_layouts_json,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    ) noexcept;


    export std::pmr::vector<vk::PipelineLayout> create_pipeline_layouts(
        vk::Device device,
        vk::AllocationCallbacks const* allocation_callbacks,
        std::span<vk::DescriptorSetLayout const> descriptor_set_layouts,
        nlohmann::json const& pipeline_layouts_json,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    ) noexcept;


    export std::pmr::vector<vk::Pipeline> create_pipeline_states(
        vk::Device device,
        vk::AllocationCallbacks const* allocation_callbacks,
        std::span<vk::ShaderModule const> shader_modules,
        std::span<vk::PipelineLayout const> pipeline_layouts,
        std::span<vk::RenderPass const> render_passes,
        nlohmann::json const& pipeline_states_json,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    ) noexcept;

    
    export struct Pipeline_resources
    {
        Pipeline_resources(
            vk::PhysicalDevice const physical_device,
            vk::PhysicalDeviceType const physical_device_type,
            vk::Device device,
            vk::Queue upload_queue,
            vk::CommandPool command_pool,
            vk::PipelineCache pipeline_cache,
            vk::PhysicalDeviceRayTracingPipelinePropertiesKHR const& physical_device_ray_tracing_properties,
            Maia::Renderer::Vulkan::Buffer_resources& shader_binding_tables_buffer_resources,
            Maia::Renderer::Vulkan::Upload_buffer const* upload_buffer,
            vk::AllocationCallbacks const* allocation_callbacks,
            nlohmann::json const& pipeline_json,
            std::filesystem::path const& pipeline_json_parent_path,
            std::pmr::polymorphic_allocator<> const& output_allocator,
            std::pmr::polymorphic_allocator<> const& temporaries_allocator
        ) noexcept;

        Pipeline_resources(Pipeline_resources const&) noexcept = delete;
        Pipeline_resources(Pipeline_resources&& other) noexcept = default;
        
        ~Pipeline_resources() noexcept;

        Pipeline_resources& operator=(Pipeline_resources const&) noexcept = delete;
        Pipeline_resources& operator=(Pipeline_resources&& other) noexcept = default;

        vk::Device device;
        vk::AllocationCallbacks const* allocation_callbacks;
        std::pmr::vector<vk::RenderPass> render_passes;
        std::pmr::vector<vk::ShaderModule> shader_modules;
        std::pmr::vector<vk::Sampler> samplers;
        std::pmr::vector<vk::DescriptorSetLayout> descriptor_set_layouts;
        std::pmr::vector<vk::PipelineLayout> pipeline_layouts;
        std::pmr::vector<vk::Pipeline> pipeline_states;
        std::pmr::vector<Maia::Renderer::Vulkan::Buffer_view> shader_binding_table_buffer_views;
        std::pmr::vector<vk::StridedDeviceAddressRegionKHR> shader_binding_tables;
        Maia::Renderer::Vulkan::Buffer_resources buffer_resources;
        std::pmr::vector<Maia::Renderer::Vulkan::Buffer_view> buffer_memory_views;
        std::pmr::vector<vk::BufferView> buffer_views;
        Maia::Renderer::Vulkan::Image_resources image_resources;
        std::pmr::vector<Maia::Renderer::Vulkan::Image_memory_view> image_memory_views;
        std::pmr::vector<vk::ImageView> image_views;
        vk::DescriptorPool descriptor_pool;
        std::pmr::vector<vk::DescriptorSet> descriptor_sets;
    };


    export struct Commands_data
    {
        std::pmr::vector<std::byte> bytes;
    };

    export Commands_data create_commands_data(
        nlohmann::json const& commands_json,
        std::span<vk::Pipeline const> pipelines,
        std::span<vk::RenderPass const> render_passes,
        std::span<vk::StridedDeviceAddressRegionKHR const> shader_binding_tables,
        std::pmr::polymorphic_allocator<std::byte> const& output_allocator,
        std::pmr::polymorphic_allocator<std::byte> const& temporaries_allocator
    ) noexcept;

    export void draw(
        vk::CommandBuffer command_buffer,
        std::span<vk::Buffer const> output_buffers,
        std::span<vk::Image const> output_images,
        std::span<vk::ImageView const> output_image_views,
        std::span<vk::ImageSubresourceRange const> output_image_subresource_ranges,
        std::span<vk::Framebuffer const> output_framebuffers,
        std::span<vk::Rect2D const> output_render_areas,
        Commands_data const& commands_data,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    ) noexcept;
}
