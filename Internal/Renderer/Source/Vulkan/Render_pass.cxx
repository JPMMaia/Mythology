export module maia.renderer.vulkan.render_pass;

import maia.renderer.vulkan.device;
import maia.renderer.vulkan.device;

import <vulkan/vulkan.h>;

import <cstdint>;
import <optional>;
import <span>;

namespace Maia::Renderer::Vulkan
{
    export struct Render_pass
    {
        VkRenderPass value;
    };

    export Render_pass create_render_pass(
        Device device,
        std::span<VkAttachmentDescription const> attachment_descriptions,
        std::span<VkSubpassDescription const> subpass_descriptions,
        std::span<VkSubpassDependency const> subpass_dependencies,
        VkAllocationCallbacks const* allocator
    ) noexcept;

    export void destroy_render_pass(
        Device device,
        Render_pass render_pass,
        VkAllocationCallbacks const* allocator
    ) noexcept;


    export VkExtent2D get_render_area_granularity(
        Device device,
        Render_pass render_pass
    ) noexcept;


    export struct Framebuffer
    {
        VkFramebuffer value;
    };

    export struct Framebuffer_dimensions
    {
        std::uint32_t width;
        std::uint32_t height;
        std::uint32_t layers;
    };

    export Framebuffer create_framebuffer(
        Device device,
        VkFramebufferCreateFlags flags,
        Render_pass render_pass,
        std::span<VkImageView const> attachments,
        Framebuffer_dimensions dimensions,
        VkAllocationCallbacks const* allocator
    ) noexcept;

    export void destroy_framebuffer(
        Device device,
        Framebuffer framebuffer,
        VkAllocationCallbacks const* allocator
    ) noexcept;
}
