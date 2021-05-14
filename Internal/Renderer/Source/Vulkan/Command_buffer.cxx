module;

#include <vulkan/vulkan.h>

#include <cstdint>
#include <memory_resource>
#include <optional>
#include <span>
#include <vector>

export module maia.renderer.vulkan.command_buffer;
namespace Maia::Renderer::Vulkan
{
    export std::pmr::vector<VkCommandBuffer> allocate_command_buffers(
        VkDevice device,
        VkCommandPool command_pool,
        VkCommandBufferLevel level,
        std::uint32_t command_buffer_count,
        VkAllocationCallbacks const* vulkan_allocator,
        std::pmr::polymorphic_allocator<> const& pmr_allocator = {}
    ) noexcept;

    export void reset_command_buffer(
        VkCommandBuffer command_buffer,
        VkCommandBufferResetFlags flags
    ) noexcept;

    export void free_command_buffers(
        VkDevice device,
        VkCommandPool command_pool,
        std::span<VkCommandBuffer const> command_buffers
    ) noexcept;

    
    export void begin_command_buffer(
        VkCommandBuffer command_buffer,
        VkCommandBufferUsageFlags flags,
        std::optional<VkCommandBufferInheritanceInfo> inheritance_info
    ) noexcept;

    export void begin_command_buffer(
        VkCommandBuffer command_buffer,
        VkCommandBufferUsageFlags flags,
        std::optional<VkCommandBufferInheritanceInfo> inheritance_info
    ) noexcept;

    export void end_command_buffer(
        VkCommandBuffer command_buffer
    ) noexcept;


    export void begin_render_pass(
        VkCommandBuffer command_buffer,
        VkRenderPass render_pass,
        VkFramebuffer framebuffer,
        VkRect2D render_area,
        std::span<VkClearValue const> clear_values,
        VkSubpassContents subpass_contents
    ) noexcept;

    export void next_subpass(
        VkCommandBuffer command_buffer,
        VkSubpassContents subpass_contents
    ) noexcept;

    export void end_render_pass(
        VkCommandBuffer command_buffer
    ) noexcept;
}