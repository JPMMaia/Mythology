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
}
