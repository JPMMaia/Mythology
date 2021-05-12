module;

#include <vulkan/vulkan.h>

#include <cstdint>
#include <optional>
#include <span>

export module maia.renderer.vulkan.queue;

import maia.renderer.vulkan.device;
namespace Maia::Renderer::Vulkan
{
    export VkQueue get_device_queue(
        VkDevice device,
        Queue_family_index queue_family_index,
        std::uint32_t queue_index
    ) noexcept;

    
    export void queue_submit(
        VkQueue queue,
        std::span<VkSemaphore const> semaphores_to_wait,
        std::span<VkPipelineStageFlags const> wait_destination_stage_mask,
        std::span<VkCommandBuffer const> command_buffers,
        std::span<VkSemaphore const> semaphores_to_signal,
        std::optional<VkFence> fence
    ) noexcept;
}