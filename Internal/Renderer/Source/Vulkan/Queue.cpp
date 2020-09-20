module maia.renderer.vulkan.queue;

import maia.renderer.vulkan.check;
import maia.renderer.vulkan.command_buffer;
import maia.renderer.vulkan.semaphore;

import <vulkan/vulkan.h>;

import <cassert>;
import <cstdint>;
import <optional>;
import <span>;

namespace Maia::Renderer::Vulkan
{
    Queue get_device_queue(
        VkDevice const device,
        Queue_family_index const queue_family_index,
        std::uint32_t const queue_index
    ) noexcept
    {
        VkQueue queue = {};
        vkGetDeviceQueue(
            device,
            queue_family_index.value,
            queue_index,
            &queue
        );

        return {queue};
    }

    void queue_submit(
        Queue queue,
        std::span<Semaphore const> const semaphores_to_wait,
        std::span<VkPipelineStageFlags const> const wait_destination_stage_mask,
        std::span<Command_buffer const> const command_buffers,
        std::span<Semaphore const> const semaphores_to_signal,
        std::optional<VkFence> const fence
    ) noexcept
    {
        assert(semaphores_to_wait.size() == wait_destination_stage_mask.size());

        static_assert(std::is_standard_layout_v<Semaphore>, "Must be standard layout so that Semaphore and Semaphore.value are pointer-interconvertible");
        static_assert(sizeof(Semaphore) == sizeof(VkSemaphore), "Semaphore must only contain VkSemaphore since using Semaphore* as a contiguous array");

        static_assert(std::is_standard_layout_v<Command_buffer>, "Must be standard layout so that Command_buffer and Command_buffer.value are pointer-interconvertible");
        static_assert(sizeof(Command_buffer) == sizeof(VkCommandBuffer), "Command_buffer must only contain VkCommandBuffer since using Command_buffer* as a contiguous array");

        VkSubmitInfo const submit_info
        {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = static_cast<uint32_t>(semaphores_to_wait.size()),
            .pWaitSemaphores = reinterpret_cast<VkSemaphore const*>(semaphores_to_wait.data()),
            .pWaitDstStageMask = wait_destination_stage_mask.data(),
            .commandBufferCount = static_cast<uint32_t>(command_buffers.size()),
            .pCommandBuffers = reinterpret_cast<VkCommandBuffer const*>(command_buffers.data()),
            .signalSemaphoreCount = static_cast<uint32_t>(semaphores_to_signal.size()),
            .pSignalSemaphores = reinterpret_cast<VkSemaphore const*>(semaphores_to_signal.data()),
        };

        check_result(
            vkQueueSubmit(
                queue.value,
                1,
                &submit_info,
                fence.has_value() ? fence->value : VK_NULL_HANDLE
            )
        );
    }
}