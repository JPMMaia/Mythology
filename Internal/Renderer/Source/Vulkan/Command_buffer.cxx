export module maia.renderer.vulkan.command_buffer;

import maia.renderer.vulkan.allocation_callbacks;
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
    export struct Command_buffer
    {
        VkCommandBuffer value;
    };

    export std::pmr::vector<Command_buffer> allocate_command_buffers(
        Device device,
        Command_pool command_pool,
        VkCommandBufferLevel level,
        std::uint32_t command_buffer_count,
        std::optional<Allocation_callbacks> vulkan_allocator,
        std::pmr::polymorphic_allocator<Command_buffer> const& pmr_allocator = {}
    ) noexcept;

    export void reset_command_buffer(
        Command_buffer command_buffer,
        VkCommandBufferResetFlags flags
    ) noexcept;

    export void free_command_buffers(
        Device device,
        Command_pool command_pool,
        std::span<Command_buffer const> command_buffers
    ) noexcept;
}