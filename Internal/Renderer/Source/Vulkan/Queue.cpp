module maia.renderer.vulkan.queue;

import maia.renderer.vulkan.check;

import <vulkan/vulkan.h>;

import <cassert>;
import <cstdint>;
import <optional>;
import <span>;

namespace Maia::Renderer::Vulkan
{
    VkQueue get_device_queue(
        VkDevice const device,
        Queue_family_index const queue_family_index,
        std::uint32_t const queue_index
    ) noexcept
    {
        VkQueue queue = {};
        vkGetDeviceQueue(
            device,
            queue_family_index,
            queue_index,
            &queue
        );

        return {queue};
    }

    void queue_submit(
        VkQueue queue,
        std::span<VkSemaphore const> const semaphores_to_wait,
        std::span<VkPipelineStageFlags const> const wait_destination_stage_mask,
        std::span<VkCommandBuffer const> const command_buffers,
        std::span<VkSemaphore const> const semaphores_to_signal,
        std::optional<VkFence> const fence
    ) noexcept
    {
        assert(semaphores_to_wait.size() == wait_destination_stage_mask.size());

        VkSubmitInfo const submit_info
        {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = static_cast<uint32_t>(semaphores_to_wait.size()),
            .pWaitSemaphores = semaphores_to_wait.data(),
            .pWaitDstStageMask = wait_destination_stage_mask.data(),
            .commandBufferCount = static_cast<uint32_t>(command_buffers.size()),
            .pCommandBuffers = command_buffers.data(),
            .signalSemaphoreCount = static_cast<uint32_t>(semaphores_to_signal.size()),
            .pSignalSemaphores = semaphores_to_signal.data(),
        };

        check_result(
            vkQueueSubmit(
                queue,
                1,
                &submit_info,
                fence.has_value() ? *fence : VK_NULL_HANDLE
            )
        );
    }
}