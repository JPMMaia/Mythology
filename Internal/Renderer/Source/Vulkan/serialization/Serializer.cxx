export module maia.renderer.vulkan.serializer;

import <nlohmann/json.hpp>;
import <vulkan/vulkan.h>;

import <cstddef>;
import <filesystem>;
import <memory_resource>;
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
        std::pmr::polymorphic_allocator<VkAttachmentDescription> const& attachments_allocator,
        std::pmr::polymorphic_allocator<VkAttachmentReference> const& attachment_reference_allocator,
        std::pmr::polymorphic_allocator<std::uint32_t> const& preserve_attachment_allocator,
        std::pmr::polymorphic_allocator<VkSubpassDescription> const& subpasses_allocator,
        std::pmr::polymorphic_allocator<VkSubpassDependency> const& dependencies_allocator
    ) noexcept;

    export std::pmr::vector<VkRenderPass> create_render_passes(
        VkDevice device,
        VkAllocationCallbacks const* allocation_callbacks,
        nlohmann::json const& render_passes_json,
        std::pmr::polymorphic_allocator<VkAttachmentDescription> const& attachments_allocator,
        std::pmr::polymorphic_allocator<VkAttachmentReference> const& attachment_reference_allocator,
        std::pmr::polymorphic_allocator<std::uint32_t> const& preserve_attachment_allocator,
        std::pmr::polymorphic_allocator<VkSubpassDescription> const& subpasses_allocator,
        std::pmr::polymorphic_allocator<VkSubpassDependency> const& dependencies_allocator,
        std::pmr::polymorphic_allocator<VkRenderPass> const& allocator
    ) noexcept;


    export std::pmr::vector<VkShaderModule> create_shader_modules(
        VkDevice const device,
        VkAllocationCallbacks const* const allocation_callbacks,
        nlohmann::json const& shader_modules_json,
        std::filesystem::path const& shaders_path,
        std::pmr::polymorphic_allocator<> const& allocator
    ) noexcept;


    export struct Commands_data
    {
        std::pmr::vector<std::byte> bytes;
    };

    export Commands_data create_commands_data(
        nlohmann::json const& commands_json,
        std::pmr::polymorphic_allocator<std::byte> const& commands_allocator
    ) noexcept;

    export void draw(
        VkCommandBuffer const command_buffer,
        VkImage const output_image,
        VkImageSubresourceRange const& output_image_subresource_range,
        Commands_data const& commands_data
    ) noexcept;
}
