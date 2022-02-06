module;

#include <vulkan/vulkan.hpp>

#include <array>
#include <memory_resource>
#include <vector>

module maia.renderer.vulkan.command_buffer;

namespace Maia::Renderer::Vulkan
{
    vk::CommandBuffer create_one_time_submit_command_buffer(
        vk::Device const device,
        vk::CommandPool const command_pool
    ) noexcept
    {
        vk::CommandBufferAllocateInfo const allocate_info
        {
            .commandPool = command_pool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1,
        };

        std::array<std::byte, sizeof(vk::CommandBuffer)> local_storage;
        std::pmr::monotonic_buffer_resource local_storage_buffer_resource{ &local_storage, local_storage.size() };
        std::pmr::polymorphic_allocator<vk::CommandBuffer> local_storage_allocator{ &local_storage_buffer_resource };
        std::pmr::vector<vk::CommandBuffer> const command_buffers = device.allocateCommandBuffers(allocate_info, local_storage_allocator);
        vk::CommandBuffer const command_buffer = command_buffers[0];

        {
            vk::CommandBufferBeginInfo const begin_info
            {
            };

            command_buffer.begin(begin_info);
        }

        return command_buffer;
    }

    void submit(
        vk::Device const device,
        vk::Queue const queue,
        vk::CommandPool const command_pool,
        vk::CommandBuffer const command_buffer,
        vk::AllocationCallbacks const* const allocation_callbacks
    )
    {
        command_buffer.end();

        vk::Fence const fence = device.createFence({}, allocation_callbacks);

        vk::SubmitInfo const submit_info
        {
            .commandBufferCount = 1,
            .pCommandBuffers = &command_buffer,
        };

        {
            vk::Result const result = queue.submit(1, &submit_info, fence);

            if (result != vk::Result::eSuccess)
            {
                throw std::runtime_error{ "Error on Vulkan submit!" };
            }
        }

        {
            vk::Result const result = device.waitForFences(1, &fence, true, std::numeric_limits<std::uint64_t>::max());

            if (result != vk::Result::eSuccess)
            {
                throw std::runtime_error{ "Timeout reached on Vulkan wait for fences!" };
            }
        }

        device.destroy(fence, allocation_callbacks);

        device.freeCommandBuffers(command_pool, 1, &command_buffer);
    }
}
