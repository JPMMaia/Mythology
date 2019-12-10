module maia.renderer.vulkan.command_buffer;

import maia.renderer.vulkan.allocation_callbacks;
import maia.renderer.vulkan.check;
import maia.renderer.vulkan.command_pool;
import maia.renderer.vulkan.device;

import <vulkan/vulkan.h>;

import <cstdint>;
import <memory_resource>;
import <optional>;
import <span>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    std::pmr::vector<Command_buffer> allocate_command_buffers(
        Device const device,
        Command_pool const command_pool,
        VkCommandBufferLevel const level,
        std::uint32_t const command_buffer_count,
        std::optional<Allocation_callbacks> const vulkan_allocator,
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
                device.value,
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
        Device const device,
        Command_pool const command_pool,
        std::span<Command_buffer const> const command_buffers
    ) noexcept
    {
        static_assert(std::is_standard_layout_v<Command_buffer>, "Must be standard layout so that Command_buffer and Command_buffer.value are pointer-interconvertible");
        static_assert(sizeof(Command_buffer) == sizeof(VkCommandBuffer), "Command_buffer must only contain VkCommandBuffer since using Command_buffer* as a contiguous array");

        vkFreeCommandBuffers(
            device.value, 
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
}