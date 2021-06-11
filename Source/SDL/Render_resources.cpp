module;

#include <vulkan/vulkan.hpp>

#include <cassert>
#include <span>
#include <utility>

module mythology.sdl.render_resources;

import maia.renderer.vulkan;

namespace Mythology::Render
{
    using namespace Maia::Renderer::Vulkan;

    Instance_resources::Instance_resources(
        Maia::Renderer::Vulkan::API_version const api_version,
        std::span<char const* const> const required_instance_extensions
    ) :
        instance{create_instance(Application_description{"Mythology", 1}, Engine_description{"Mythology Engine", 1}, api_version, {}, required_instance_extensions, nullptr)}
    {
    }
    Instance_resources::~Instance_resources() noexcept
    {
        if (this->instance != VK_NULL_HANDLE)
        {
            Maia::Renderer::Vulkan::destroy_instance(this->instance);
        }
    }

    Instance_resources::Instance_resources(Instance_resources&& other) noexcept :
        instance{std::exchange(other.instance, VkInstance{VK_NULL_HANDLE})}
    {
    }

    Instance_resources& Instance_resources::operator=(Instance_resources&& other) noexcept
    {
        if (this->instance != VK_NULL_HANDLE)
        {
            Maia::Renderer::Vulkan::destroy_instance(this->instance);
        }

        this->instance = std::exchange(other.instance, VkInstance{VK_NULL_HANDLE});

        return *this;
    }


    namespace
    {
        std::pmr::vector<VkSemaphore> create_semaphores(
            std::span<VkDevice const> const devices,
            VkSemaphoreType const semaphore_type,
            Semaphore_value const initial_value,
            VkSemaphoreCreateFlags const flags,
            VkAllocationCallbacks const* const vulkan_allocator,
            std::pmr::polymorphic_allocator<> const& vector_allocator
        )
        {
            std::pmr::vector<VkSemaphore> semaphores{vector_allocator};
            semaphores.resize(devices.size());

            for (std::size_t index = 0; index < devices.size(); ++index)
            {
                semaphores[index] = 
                    create_semaphore(
                        devices[index],
                        semaphore_type,
                        initial_value,
                        flags,
                        vulkan_allocator
                    );
            }

            return semaphores;
        }

        std::pmr::vector<VkFence> create_fences(
            std::span<VkDevice const> const devices,
            VkFenceCreateFlags const flags,
            VkAllocationCallbacks const* const vulkan_allocator,
            std::pmr::polymorphic_allocator<> const& vector_allocator
        )
        {
            std::pmr::vector<VkFence> fences{vector_allocator};
            fences.resize(devices.size());

            for (std::size_t index = 0; index < devices.size(); ++index)
            {
                fences[index] = 
                    create_fence(
                        devices[index],
                        flags,
                        vulkan_allocator
                    );
            }

            return fences;
        }

        std::pmr::vector<Frame_synchronization_resources> create_frames_synchronization_resources(
            std::size_t const number_of_frames,
            std::span<VkDevice const> devices,
            std::pmr::polymorphic_allocator<> const& allocator
        )
        {
            auto const create_resources = [&allocator, devices] () -> Frame_synchronization_resources
            {
                return
                {
                    .available_frame_semaphores = create_semaphores(devices, VK_SEMAPHORE_TYPE_BINARY, {0}, {}, nullptr, allocator),
                    .finished_frame_semaphores = create_semaphores(devices, VK_SEMAPHORE_TYPE_BINARY, {0}, {}, nullptr, allocator),
                    .available_frame_fences = create_fences(devices, VK_FENCE_CREATE_SIGNALED_BIT, nullptr, allocator),
                };
            };

            std::pmr::vector<Frame_synchronization_resources> frames{allocator};
            frames.resize(number_of_frames);

            std::generate_n(
                frames.begin(),
                number_of_frames,
                create_resources
            );

            return frames;
        }
    }


    Synchronization_resources::Synchronization_resources(
        std::size_t const number_of_frames,
        std::span<VkDevice const> devices,
        std::pmr::polymorphic_allocator<> const& allocator
    ) :
        devices{devices.begin(), devices.end(), allocator},
        frames{create_frames_synchronization_resources(number_of_frames, devices, allocator)}
    {
    }

    Synchronization_resources::Synchronization_resources(Synchronization_resources&& other) noexcept :
        devices{std::move(other.devices)},
        frames{std::move(other.frames)}
    {
    }
    
    Synchronization_resources::~Synchronization_resources() noexcept
    {
        for (std::size_t index = 0; index < this->devices.size(); ++index)
        {
            for (Frame_synchronization_resources const& frame_resources : this->frames)
            {
                destroy_fence(this->devices[index], frame_resources.available_frame_fences[index], {});
                destroy_semaphore(this->devices[index], frame_resources.finished_frame_semaphores[index], {});
                destroy_semaphore(this->devices[index], frame_resources.available_frame_semaphores[index], {});
            }
        }
    }

    Synchronization_resources& Synchronization_resources::operator=(Synchronization_resources&& other) noexcept
    {
        std::swap(this->devices, other.devices);
        std::swap(this->frames, other.frames);

        return *this;
    }

    namespace
    {
        std::pmr::vector<VkCommandPool> create_command_pools(
            std::span<VkDevice const> devices,
            std::span<VkCommandPoolCreateFlags const> flags,
            std::span<std::uint32_t> const queue_family_indices,
            std::pmr::polymorphic_allocator<> const& allocator
        )
        {
            assert(devices.size() == flags.size());
            assert(devices.size() == queue_family_indices.size());

            std::pmr::vector<VkCommandPool> command_pools{allocator};
            command_pools.reserve(queue_family_indices.size());

            for (std::size_t index = 0; index < queue_family_indices.size(); ++index)
            {
                VkCommandPool const command_pool = create_command_pool(devices[index], flags[index], {queue_family_indices[index]}, {});

                command_pools.push_back(command_pool);
            }

            return command_pools;
        }
    }

    Command_pools_resources::Command_pools_resources(
        std::span<VkDevice const> devices,
        std::span<VkCommandPoolCreateFlags const> flags,
        std::span<std::uint32_t> const queue_family_indices,
        std::pmr::polymorphic_allocator<> const& allocator
    ) noexcept :
        devices{devices.begin(), devices.end(), allocator},
        command_pools{create_command_pools(devices, flags, queue_family_indices, allocator)}
    {
    }

    Command_pools_resources::Command_pools_resources(Command_pools_resources&& other) noexcept :
        devices{std::move(other.devices)},
        command_pools{std::move(other.command_pools)}
    {
    }

    Command_pools_resources::~Command_pools_resources() noexcept
    {
        for (std::size_t index = 0; index < this->command_pools.size(); ++index)
        {
            destroy_command_pool(this->devices[index], this->command_pools[index], {});
        }
    }

    Command_pools_resources& Command_pools_resources::operator=(Command_pools_resources&& other) noexcept
    {
        std::swap(this->devices, other.devices);
        std::swap(this->command_pools, other.command_pools);

        return *this;
    }


    std::pmr::vector<std::pmr::vector<VkCommandBuffer>> allocate_command_buffers(
        std::span<VkDevice const> const devices,
        std::span<VkCommandPool const> const command_pools,
        VkCommandBufferLevel const level,
        std::uint32_t const number_of_frames_in_flight,
        VkAllocationCallbacks const* const vulkan_allocator,
        std::pmr::polymorphic_allocator<> const& pmr_allocator
    )
    {
        using Maia::Renderer::Vulkan::check_result;

        assert(devices.size() == command_pools.size());

        auto const create_command_buffer = [=] (std::size_t const index) -> VkCommandBuffer
        {
            VkDevice const device = devices[index];
            VkCommandPool const command_pool = command_pools[index];

            VkCommandBufferAllocateInfo const allocate_info
            {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .pNext = nullptr,
                .commandPool = command_pool,
                .level = level,
                .commandBufferCount = 1,
            };

            VkCommandBuffer command_buffer = VK_NULL_HANDLE;
            check_result(
                vkAllocateCommandBuffers(
                    device,
                    &allocate_info,
                    &command_buffer
                )
            );

            return command_buffer;
        };

        auto const create_frame_command_buffers = [=] () -> std::pmr::vector<VkCommandBuffer>
        {
            std::pmr::vector<VkCommandBuffer> frame_command_buffers{pmr_allocator};
            frame_command_buffers.reserve(command_pools.size());

            for (std::size_t index = 0; index < command_pools.size(); ++index)
            {
                VkCommandBuffer const command_buffer = create_command_buffer(index);

                frame_command_buffers.push_back(command_buffer);
            }

            return frame_command_buffers;
        };
    
        std::pmr::vector<std::pmr::vector<VkCommandBuffer>> command_buffers{pmr_allocator};
        command_buffers.resize(number_of_frames_in_flight);

        std::generate_n(command_buffers.begin(), number_of_frames_in_flight, create_frame_command_buffers);

        return command_buffers;
    }
}
