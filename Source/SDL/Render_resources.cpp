module;

#include <vulkan/vulkan.hpp>

#include <cassert>
#include <span>
#include <utility>

module mythology.sdl.render_resources;

namespace Mythology::Render
{
    vk::Instance create_instance(
        std::uint32_t const api_version,
        std::span<char const* const> const required_instance_extensions
    )
    {
        vk::ApplicationInfo const application_info
        {
            .pApplicationName = "Mythology",
            .applicationVersion = 1,
            .pEngineName = "Mythology Engine",
            .engineVersion = 1,
            .apiVersion = api_version,
        };

        vk::InstanceCreateInfo const instance_create_info
        {
            .flags = {},
            .pApplicationInfo = &application_info,
            .enabledLayerCount = {},
            .ppEnabledLayerNames = {},
            .enabledExtensionCount = static_cast<std::uint32_t>(required_instance_extensions.size()),
            .ppEnabledExtensionNames = required_instance_extensions.data(),
        };

        return vk::createInstance(instance_create_info);
    }

    Instance_resources::Instance_resources(
        std::uint32_t const api_version,
        std::span<char const* const> const required_instance_extensions
    ) :
        instance{create_instance(api_version, required_instance_extensions)}
    {
    }
    Instance_resources::~Instance_resources() noexcept
    {
        if (this->instance != vk::Instance{})
        {
            this->instance.destroy();
        }
    }

    Instance_resources::Instance_resources(Instance_resources&& other) noexcept :
        instance{std::exchange(other.instance, VkInstance{})}
    {
    }

    Instance_resources& Instance_resources::operator=(Instance_resources&& other) noexcept
    {
        std::swap(this->instance, other.instance);

        return *this;
    }


    namespace
    {
        std::pmr::vector<vk::Semaphore> create_semaphores(
            std::span<vk::Device const> const devices,
            vk::SemaphoreType const semaphore_type,
            std::uint64_t const initial_value,
            vk::SemaphoreCreateFlags const flags,
            vk::AllocationCallbacks const* const vulkan_allocator,
            std::pmr::polymorphic_allocator<> const& vector_allocator
        )
        {
            std::pmr::vector<vk::Semaphore> semaphores{vector_allocator};
            semaphores.resize(devices.size());

            for (std::size_t index = 0; index < devices.size(); ++index)
            {
                vk::Device const device = devices[index];

                vk::SemaphoreTypeCreateInfo const type_create_info
                {
                    .semaphoreType = semaphore_type,
                    .initialValue = initial_value
                };

                vk::SemaphoreCreateInfo const create_info
                {
                    .pNext = nullptr,
                    .flags = flags
                };

                semaphores[index] = device.createSemaphore(
                    create_info,
                    vulkan_allocator
                );
            }

            return semaphores;
        }

        std::pmr::vector<vk::Fence> create_fences(
            std::span<vk::Device const> const devices,
            vk::FenceCreateFlags const flags,
            vk::AllocationCallbacks const* const vulkan_allocator,
            std::pmr::polymorphic_allocator<> const& vector_allocator
        )
        {
            std::pmr::vector<vk::Fence> fences{vector_allocator};
            fences.resize(devices.size());

            for (std::size_t index = 0; index < devices.size(); ++index)
            {
                vk::Device const device = devices[index];

                vk::FenceCreateInfo const create_info
                {
                    .flags = flags
                };

                fences[index] = 
                    device.createFence(
                        create_info,
                        vulkan_allocator
                    );
            }

            return fences;
        }

        std::pmr::vector<Frame_synchronization_resources> create_frames_synchronization_resources(
            std::size_t const number_of_frames,
            std::span<vk::Device const> devices,
            std::pmr::polymorphic_allocator<> const& allocator
        )
        {
            auto const create_resources = [&allocator, devices] () -> Frame_synchronization_resources
            {
                return
                {
                    .available_frame_semaphores = create_semaphores(devices, vk::SemaphoreType::eBinary, 0, {}, nullptr, allocator),
                    .finished_frame_semaphores = create_semaphores(devices, vk::SemaphoreType::eBinary, 0, {}, nullptr, allocator),
                    .available_frame_fences = create_fences(devices, vk::FenceCreateFlagBits::eSignaled, nullptr, allocator),
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
        std::span<vk::Device const> devices,
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
                vk::Device const device = this->devices[index];

                device.destroy(frame_resources.available_frame_fences[index], {});
                device.destroy(frame_resources.finished_frame_semaphores[index], {});
                device.destroy(frame_resources.available_frame_semaphores[index], {});
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
        std::pmr::vector<vk::CommandPool> create_command_pools(
            std::span<vk::Device const> devices,
            std::span<vk::CommandPoolCreateFlags const> flags,
            std::span<std::uint32_t> const queue_family_indices,
            std::pmr::polymorphic_allocator<> const& allocator
        )
        {
            assert(devices.size() == flags.size());
            assert(devices.size() == queue_family_indices.size());

            std::pmr::vector<vk::CommandPool> command_pools{allocator};
            command_pools.reserve(queue_family_indices.size());

            for (std::size_t index = 0; index < queue_family_indices.size(); ++index)
            {
                vk::Device const device = devices[index];

                vk::CommandPoolCreateInfo const create_info
                {
                    .flags = flags[index],
                    .queueFamilyIndex = queue_family_indices[index],
                };

                vk::CommandPool const command_pool = device.createCommandPool(
                    create_info
                );

                command_pools.push_back(command_pool);
            }

            return command_pools;
        }
    }

    Command_pools_resources::Command_pools_resources(
        std::span<vk::Device const> devices,
        std::span<vk::CommandPoolCreateFlags const> flags,
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
            vk::Device const device = this->devices[index];

            device.destroy(this->command_pools[index], {});
        }
    }

    Command_pools_resources& Command_pools_resources::operator=(Command_pools_resources&& other) noexcept
    {
        std::swap(this->devices, other.devices);
        std::swap(this->command_pools, other.command_pools);

        return *this;
    }


    std::pmr::vector<std::pmr::vector<vk::CommandBuffer>> allocate_command_buffers(
        std::span<vk::Device const> const devices,
        std::span<vk::CommandPool const> const command_pools,
        vk::CommandBufferLevel const level,
        std::uint32_t const number_of_frames_in_flight,
        vk::AllocationCallbacks const* const vulkan_allocator,
        std::pmr::polymorphic_allocator<> const& pmr_allocator
    )
    {
        assert(devices.size() == command_pools.size());

        auto const create_command_buffer = [=] (std::size_t const index) -> vk::CommandBuffer
        {
            vk::Device const device = devices[index];
            vk::CommandPool const command_pool = command_pools[index];

            vk::CommandBufferAllocateInfo const allocate_info
            {
                .commandPool = command_pool,
                .level = level,
                .commandBufferCount = 1,
            };

            vk::CommandBuffer command_buffer = {};
            vk::Result const result = device.allocateCommandBuffers(
                &allocate_info,
                &command_buffer
            );

            if (result != vk::Result::eSuccess)
            {
                vk::throwResultException(result, "vk::Device::allocateCommandBuffers");
            }

            return command_buffer;
        };

        auto const create_frame_command_buffers = [=] () -> std::pmr::vector<vk::CommandBuffer>
        {
            std::pmr::vector<vk::CommandBuffer> frame_command_buffers{pmr_allocator};
            frame_command_buffers.reserve(command_pools.size());

            for (std::size_t index = 0; index < command_pools.size(); ++index)
            {
                vk::CommandBuffer const command_buffer = create_command_buffer(index);

                frame_command_buffers.push_back(command_buffer);
            }

            return frame_command_buffers;
        };
    
        std::pmr::vector<std::pmr::vector<vk::CommandBuffer>> command_buffers{pmr_allocator};
        command_buffers.resize(number_of_frames_in_flight);

        std::generate_n(command_buffers.begin(), number_of_frames_in_flight, create_frame_command_buffers);

        return command_buffers;
    }
}
