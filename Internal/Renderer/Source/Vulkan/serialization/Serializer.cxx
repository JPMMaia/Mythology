export module maia.renderer.vulkan.serializer;

import <nlohmann/json.hpp>;
import <vulkan/vulkan.h>;

import <cstddef>;
import <filesystem>;
import <memory_resource>;
import <span>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    export struct Render_pass_create_info_resources
    {
        std::pmr::vector<VkAttachmentDescription> attachments;
        std::pmr::vector<VkAttachmentReference> attachment_references;
        std::pmr::vector<std::uint32_t> preserve_attachments;
        std::pmr::vector<VkSubpassDescription> subpasses;
        std::pmr::vector<VkSubpassDependency> dependencies;
        VkRenderPassCreateInfo create_info;
    };

    export Render_pass_create_info_resources create_render_pass_create_info_resources(
        nlohmann::json const& render_pass_json,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    ) noexcept;

    export std::pmr::vector<VkRenderPass> create_render_passes(
        VkDevice device,
        VkAllocationCallbacks const* allocation_callbacks,
        nlohmann::json const& render_passes_json,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    ) noexcept;


    export std::pmr::vector<VkShaderModule> create_shader_modules(
        VkDevice const device,
        VkAllocationCallbacks const* const allocation_callbacks,
        nlohmann::json const& shader_modules_json,
        std::filesystem::path const& shaders_path,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    ) noexcept;


    export std::pmr::vector<VkSampler> create_samplers(
        VkDevice device,
        VkAllocationCallbacks const* allocation_callbacks,
        nlohmann::json const& samplers_json,
        std::pmr::polymorphic_allocator<> const& output_allocator
    ) noexcept;


    export std::pmr::vector<VkDescriptorSetLayout> create_descriptor_set_layouts(
        VkDevice device,
        VkAllocationCallbacks const* allocation_callbacks,
        std::span<VkSampler const> samplers,
        nlohmann::json const& descriptor_set_layouts_json,
        std::pmr::polymorphic_allocator<> const& output_allocator
    ) noexcept;


    export std::pmr::vector<VkPipelineLayout> create_pipeline_layouts(
        VkDevice device,
        VkAllocationCallbacks const* allocation_callbacks,
        std::span<VkDescriptorSetLayout const> descriptor_set_layouts,
        nlohmann::json const& pipeline_layouts_json,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    ) noexcept;


    export std::pmr::vector<VkPipeline> create_pipeline_states(
        VkDevice device,
        VkAllocationCallbacks const* allocation_callbacks,
        std::span<VkShaderModule const> shader_modules,
        std::span<VkPipelineLayout const> pipeline_layouts,
        std::span<VkRenderPass const> render_passes,
        nlohmann::json const& pipeline_states_json,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    ) noexcept;


    export struct Commands_data
    {
        std::pmr::vector<std::byte> bytes;
    };

    export Commands_data create_commands_data(
        nlohmann::json const& commands_json,
        std::span<VkPipeline const> pipelines,
        std::span<VkRenderPass const> render_passes,
        std::pmr::polymorphic_allocator<std::byte> const& output_allocator,
        std::pmr::polymorphic_allocator<std::byte> const& temporaries_allocator
    ) noexcept;

    export void draw(
        VkCommandBuffer command_buffer,
        VkImage output_image,
        VkImageSubresourceRange const& output_image_subresource_range,
        VkFramebuffer output_framebuffer,
        VkRect2D output_render_area,
        Commands_data const& commands_data,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    ) noexcept;
}
