module maia.renderer.vulkan.render_pass;

import maia.renderer.vulkan.check;
import maia.renderer.vulkan.device;

import <vulkan/vulkan.h>;

import <optional>;
import <span>;

namespace Maia::Renderer::Vulkan
{
    Render_pass create_render_pass(
        Device const device,
        std::span<VkAttachmentDescription const> const attachment_descriptions,
        std::span<VkSubpassDescription const> const subpass_descriptions,
        std::span<VkSubpassDependency const> const subpass_dependencies,
        VkAllocationCallbacks const* const allocator
    ) noexcept
    {
        VkRenderPassCreateInfo const create_info
        {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .attachmentCount = static_cast<uint32_t>(attachment_descriptions.size()),
            .pAttachments = attachment_descriptions.data(),
            .subpassCount = static_cast<uint32_t>(subpass_descriptions.size()),
            .pSubpasses = subpass_descriptions.data(),
            .dependencyCount = static_cast<uint32_t>(subpass_dependencies.size()),
            .pDependencies = subpass_dependencies.data(),
        };

        VkRenderPass render_pass = {};
        check_result(
            vkCreateRenderPass(
                device.value,
                &create_info,
                allocator,
                &render_pass
            )
        );

        return {render_pass};
    }

    void destroy_render_pass(
        Device const device,
        Render_pass const render_pass,
        VkAllocationCallbacks const* const allocator
    ) noexcept
    {
        vkDestroyRenderPass(
            device.value,
            render_pass.value,
            allocator
        );
    }


    VkExtent2D get_render_area_granularity(
        Device device,
        Render_pass render_pass
    ) noexcept
    {
        VkExtent2D granularity = {};
        vkGetRenderAreaGranularity(
            device.value,
            render_pass.value,
            &granularity
        );
        return granularity;
    }


    Framebuffer create_framebuffer(
        Device const device,
        VkFramebufferCreateFlags const flags,
        Render_pass const render_pass,
        std::span<VkImageView const> const attachments,
        Framebuffer_dimensions const dimensions,
        VkAllocationCallbacks const* const allocator
    ) noexcept
    {
        VkFramebufferCreateInfo const create_info
        {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = flags,
            .renderPass = render_pass.value,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .width = dimensions.width,
            .height = dimensions.height,
            .layers = dimensions.layers
        };

        VkFramebuffer framebuffer = {};
        check_result(
            vkCreateFramebuffer(
                device.value,
                &create_info,
                allocator,
                &framebuffer
            )
        );

        return {framebuffer};
    }

    void destroy_framebuffer(
        Device const device,
        Framebuffer const framebuffer,
        VkAllocationCallbacks const* const allocator
    ) noexcept
    {
        vkDestroyFramebuffer(
            device.value, 
            framebuffer.value,
            allocator
        );
    }
}
