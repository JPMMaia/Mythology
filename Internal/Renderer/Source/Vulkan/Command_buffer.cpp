module maia.renderer.vulkan.command_buffer;

import maia.renderer.vulkan.check;
import maia.renderer.vulkan.command_pool;
import maia.renderer.vulkan.render_pass;

import <vulkan/vulkan.h>;

import <cstdint>;
import <memory_resource>;
import <optional>;
import <span>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    std::pmr::vector<Command_buffer> allocate_command_buffers(
        VkDevice const device,
        Command_pool const command_pool,
        VkCommandBufferLevel const level,
        std::uint32_t const command_buffer_count,
        VkAllocationCallbacks const* const vulkan_allocator,
        std::pmr::polymorphic_allocator<Command_buffer> const& pmr_allocator
    ) noexcept
    {
        VkCommandBufferAllocateInfo const allocate_info
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = command_pool.value,
            .level = level,
            .commandBufferCount = command_buffer_count,
        };

        std::pmr::vector<Command_buffer> command_buffers{command_buffer_count, pmr_allocator};

        static_assert(std::is_standard_layout_v<Command_buffer>, "Must be standard layout so that Command_buffer and Command_buffer.value are pointer-interconvertible");
        static_assert(sizeof(Command_buffer) == sizeof(VkCommandBuffer), "Command_buffer must only contain VkCommandBuffer since using Command_buffer* as a contiguous array");
        check_result(
            vkAllocateCommandBuffers(
                device,
                &allocate_info,
                reinterpret_cast<VkCommandBuffer*>(command_buffers.data())
            )
        );

        return command_buffers;
    }

    void reset_command_buffer(
        Command_buffer const command_buffer,
        VkCommandBufferResetFlags const flags
    ) noexcept
    {
        check_result(
            vkResetCommandBuffer(command_buffer.value, flags)
        );
    }

    void free_command_buffers(
        VkDevice const device,
        Command_pool const command_pool,
        std::span<Command_buffer const> const command_buffers
    ) noexcept
    {
        static_assert(std::is_standard_layout_v<Command_buffer>, "Must be standard layout so that Command_buffer and Command_buffer.value are pointer-interconvertible");
        static_assert(sizeof(Command_buffer) == sizeof(VkCommandBuffer), "Command_buffer must only contain VkCommandBuffer since using Command_buffer* as a contiguous array");

        vkFreeCommandBuffers(
            device, 
            command_pool.value, 
            static_cast<uint32_t>(command_buffers.size()),
            reinterpret_cast<VkCommandBuffer const*>(command_buffers.data())
        );
    }


    void begin_command_buffer(
        Command_buffer const command_buffer,
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
                command_buffer.value,
                &begin_info
            )
        );
    }

    void end_command_buffer(
        Command_buffer command_buffer
    ) noexcept
    {
        check_result(
            vkEndCommandBuffer(command_buffer.value)
        );
    }


    void begin_render_pass(
        Command_buffer const command_buffer,
        Render_pass const render_pass,
        Framebuffer const framebuffer,
        VkRect2D const render_area,
        std::span<VkClearValue const> const clear_values,
        VkSubpassContents const subpass_contents
    ) noexcept
    {
        VkRenderPassBeginInfo const render_pass_begin
        {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = render_pass.value,
            .framebuffer = framebuffer.value,
            .renderArea = render_area,
            .clearValueCount = static_cast<uint32_t>(clear_values.size()),
            .pClearValues = clear_values.data(),
        };

        vkCmdBeginRenderPass(
            command_buffer.value, 
            &render_pass_begin,
            subpass_contents
        );
    }

    void next_subpass(
        Command_buffer const command_buffer,
        VkSubpassContents const subpass_contents
    ) noexcept
    {
        vkCmdNextSubpass(
            command_buffer.value,
            subpass_contents
        );
    }

    void end_render_pass(
        Command_buffer const command_buffer
    ) noexcept
    {
        vkCmdEndRenderPass(
            command_buffer.value
        );
    }
}