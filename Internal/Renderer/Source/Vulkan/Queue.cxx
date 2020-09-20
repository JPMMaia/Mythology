export module maia.renderer.vulkan.queue;

import maia.renderer.vulkan.command_buffer;
import maia.renderer.vulkan.semaphore;

import <vulkan/vulkan.h>;

import <cstdint>;
import <optional>;
import <span>;

namespace Maia::Renderer::Vulkan
{
    export struct Queue
    {
        VkQueue value = VK_NULL_HANDLE;
    };

    export Queue get_device_queue(
        VkDevice device,
        Queue_family_index queue_family_index,
        std::uint32_t queue_index
    ) noexcept;

    
    export void queue_submit(
        Queue queue,
        std::span<Semaphore const> semaphores_to_wait,
        std::span<VkPipelineStageFlags const> wait_destination_stage_mask,
        std::span<Command_buffer const> command_buffers,
        std::span<Semaphore const> semaphores_to_signal,
        std::optional<VkFence> fence
    ) noexcept;
}