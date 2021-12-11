module;

#include <vulkan/vulkan.hpp>

#include <array>
#include <memory_resource>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

module maia.renderer.vulkan.upload;

import maia.renderer.vulkan.buffer_resources;

namespace Maia::Renderer::Vulkan
{
    Upload_buffer::Upload_buffer(
        vk::Device const device,
        Buffer_resources& buffer_resources,
        vk::DeviceSize const size
    ) :
        m_device{ device },
        m_buffer_view{ buffer_resources.allocate_buffer(size, 1, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent) },
        m_mapped_data{ device.mapMemory(m_buffer_view.memory, m_buffer_view.offset, m_buffer_view.size, vk::MemoryMapFlags{}) }
    {
    }

    Upload_buffer::Upload_buffer(Upload_buffer&& other) noexcept :
        m_device{ other.m_device },
        m_buffer_view{ std::exchange(other.m_buffer_view, Buffer_view{}) },
        m_mapped_data{ other.m_mapped_data }
    {
    }

    Upload_buffer::~Upload_buffer()
    {
        if (m_buffer_view.memory != vk::DeviceMemory{})
        {
            m_device.unmapMemory(m_buffer_view.memory);
        }
    }

    Upload_buffer& Upload_buffer::operator=(Upload_buffer&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_buffer_view, other.m_buffer_view);
        std::swap(m_mapped_data, other.m_mapped_data);

        return *this;
    }

    Buffer_view Upload_buffer::buffer_view() const noexcept
    {
        return m_buffer_view;
    }

    void* Upload_buffer::mapped_data() const noexcept
    {
        return m_mapped_data;
    }


    namespace
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
        ) noexcept
        {
            command_buffer.end();

            vk::Fence const fence = device.createFence({}, allocation_callbacks);

            vk::SubmitInfo const submit_info
            {
                .commandBufferCount = 1,
                .pCommandBuffers = &command_buffer,
            };

            queue.submit(1, &submit_info, fence);

            device.waitForFences(1, &fence, true, std::numeric_limits<std::uint64_t>::max());

            device.destroy(fence, allocation_callbacks);

            device.freeCommandBuffers(command_pool, 1, &command_buffer);
        }
    }

    void upload_data(
        vk::Device const device,
        vk::Queue const queue,
        vk::CommandPool const command_pool,
        Buffer_view const& buffer_view,
        std::span<std::byte const> const data,
        Upload_buffer const* const upload_buffer,
        vk::AllocationCallbacks const* const allocation_callbacks
    )
    {
        if (upload_buffer == nullptr)
        {
            void* const mapped_data = device.mapMemory(
                buffer_view.memory,
                buffer_view.offset,
                buffer_view.size,
                vk::MemoryMapFlags{}
            );

            std::memcpy(
                mapped_data,
                data.data(),
                data.size()
            );

            device.unmapMemory(
                buffer_view.memory
            );
        }
        else
        {
            Buffer_view const upload_buffer_view = upload_buffer->buffer_view();

            if (data.size() > upload_buffer_view.size)
            {
                throw std::out_of_range{ "Size of data to upload is bigger than the upload buffer size!" };
            }

            std::memcpy(
                upload_buffer->mapped_data(),
                data.data(),
                data.size()
            );

            {
                vk::CommandBuffer const command_buffer = create_one_time_submit_command_buffer(device, command_pool);

                {
                    vk::BufferCopy const buffer_copy
                    {
                        .srcOffset = upload_buffer_view.offset,
                        .dstOffset = buffer_view.offset,
                        .size = data.size(),
                    };

                    command_buffer.copyBuffer(
                        upload_buffer_view.buffer,
                        buffer_view.buffer,
                        1,
                        &buffer_copy
                    );
                }

                submit(device, queue, command_pool, command_buffer, allocation_callbacks);
            }
        }
    }
}
