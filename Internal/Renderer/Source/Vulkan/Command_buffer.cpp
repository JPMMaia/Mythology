module maia.renderer.vulkan.command_buffer;

import maia.renderer.vulkan.check;

import <vulkan/vulkan.h>;

import <cstdint>;
import <memory_resource>;
import <optional>;
import <span>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    std::pmr::vector<VkCommandBuffer> allocate_command_buffers(
        VkDevice const device,
        VkCommandPool const command_pool,
        VkCommandBufferLevel const level,
        std::uint32_t const command_buffer_count,
        VkAllocationCallbacks const* const vulkan_allocator,
        std::pmr::polymorphic_allocator<VkCommandBuffer> const& pmr_allocator
    ) noexcept
    {
        VkCommandBufferAllocateInfo const allocate_info
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = command_pool,
            .level = level,
            .commandBufferCount = command_buffer_count,
        };

        std::pmr::vector<VkCommandBuffer> command_buffers{command_buffer_count, pmr_allocator};
        check_result(
            vkAllocateCommandBuffers(
                device,
                &allocate_info,
                command_buffers.data()
            )
        );

        return command_buffers;
    }

    void reset_command_buffer(
        VkCommandBuffer const command_buffer,
        VkCommandBufferResetFlags const flags
    ) noexcept
    {
        check_result(
            vkResetCommandBuffer(command_buffer, flags)
        );
    }

    void free_command_buffers(
        VkDevice const device,
        VkCommandPool const command_pool,
        std::span<VkCommandBuffer const> const command_buffers
    ) noexcept
    {
        vkFreeCommandBuffers(
            device, 
            command_pool, 
            static_cast<uint32_t>(command_buffers.size()),
            command_buffers.data()
        );
    }


    void begin_command_buffer(
        VkCommandBuffer const command_buffer,
        VkCommandBufferUsageFlags const flags,
        std::optional<VkCommandBufferInheritanceInfo> const inheritance_info
    ) noexcept
    {
        VkCommandBufferBeginInfo const begin_info
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = flags,
            .pInheritanceInfo = inheritance_info.has_value() ? &inheritance_info.value() : nullptr
        };

        check_result(
            vkBeginCommandBuffer(
                command_buffer,
                &begin_info
            )
        );
    }

    void end_command_buffer(
        VkCommandBuffer command_buffer
    ) noexcept
    {
        check_result(
            vkEndCommandBuffer(command_buffer)
        );
    }


    void begin_render_pass(
        VkCommandBuffer const command_buffer,
        VkRenderPass const render_pass,
        VkFramebuffer const framebuffer,
        VkRect2D const render_area,
        std::span<VkClearValue const> const clear_values,
        VkSubpassContents const subpass_contents
    ) noexcept
    {
        VkRenderPassBeginInfo const render_pass_begin
        {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = render_pass,
            .framebuffer = framebuffer,
            .renderArea = render_area,
            .clearValueCount = static_cast<uint32_t>(clear_values.size()),
            .pClearValues = clear_values.data(),
        };

        vkCmdBeginRenderPass(
            command_buffer, 
            &render_pass_begin,
            subpass_contents
        );
    }

    void next_subpass(
        VkCommandBuffer const command_buffer,
        VkSubpassContents const subpass_contents
    ) noexcept
    {
        vkCmdNextSubpass(
            command_buffer,
            subpass_contents
        );
    }

    void end_render_pass(
        VkCommandBuffer const command_buffer
    ) noexcept
    {
        vkCmdEndRenderPass(
            command_buffer
        );
    }
}