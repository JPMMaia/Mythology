export module maia.renderer.vulkan.render_pass;

import <vulkan/vulkan.h>;

import <cstdint>;
import <optional>;
import <span>;

namespace Maia::Renderer::Vulkan
{
    export VkRenderPass create_render_pass(
        VkDevice device,
        std::span<VkAttachmentDescription const> attachment_descriptions,
        std::span<VkSubpassDescription const> subpass_descriptions,
        std::span<VkSubpassDependency const> subpass_dependencies,
        VkAllocationCallbacks const* allocator
    ) noexcept;

    export void destroy_render_pass(
        VkDevice device,
        VkRenderPass render_pass,
        VkAllocationCallbacks const* allocator
    ) noexcept;


    export VkExtent2D get_render_area_granularity(
        VkDevice device,
        VkRenderPass render_pass
    ) noexcept;


    export struct Framebuffer_dimensions
    {
        std::uint32_t width;
        std::uint32_t height;
        std::uint32_t layers;
    };

    export VkFramebuffer create_framebuffer(
        VkDevice device,
        VkFramebufferCreateFlags flags,
        VkRenderPass render_pass,
        std::span<VkImageView const> attachments,
        Framebuffer_dimensions dimensions,
        VkAllocationCallbacks const* allocator
    ) noexcept;

    export void destroy_framebuffer(
        VkDevice device,
        VkFramebuffer framebuffer,
        VkAllocationCallbacks const* allocator
    ) noexcept;
}
