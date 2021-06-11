module;

#include <vulkan/vulkan.hpp>

#include <memory_resource>
#include <optional>
#include <span>
#include <vector>

export module mythology.sdl.render_resources;

import maia.renderer.vulkan;

namespace Mythology::Render
{
    export struct Instance_resources
    {
        Instance_resources(
            Maia::Renderer::Vulkan::API_version api_version,
            std::span<char const* const> required_instance_extensions
        );
        Instance_resources(Instance_resources const&) = delete;
        Instance_resources(Instance_resources&& other) noexcept;
        ~Instance_resources() noexcept;

        Instance_resources& operator=(Instance_resources const&) = delete;
        Instance_resources& operator=(Instance_resources&& other) noexcept;

        VkInstance instance = VK_NULL_HANDLE;
    };

    export struct Frame_synchronization_resources
    {
        std::pmr::vector<VkSemaphore> available_frame_semaphores;
        std::pmr::vector<VkSemaphore> finished_frame_semaphores;
        std::pmr::vector<VkFence> available_frame_fences;
    };

    export struct Synchronization_resources
    {
        Synchronization_resources(
            std::size_t number_of_frames,
            std::span<VkDevice const> devices,
            std::pmr::polymorphic_allocator<> const& allocator
        );
        Synchronization_resources(Synchronization_resources const&) = delete;
        Synchronization_resources(Synchronization_resources&& other) noexcept;
        ~Synchronization_resources() noexcept;

        Synchronization_resources& operator=(Synchronization_resources const&) = delete;
        Synchronization_resources& operator=(Synchronization_resources&& other) noexcept;

        std::pmr::vector<VkDevice> devices;
        std::pmr::vector<Frame_synchronization_resources> frames;
    };

    export struct Command_pools_resources
    {
        Command_pools_resources(
            std::span<VkDevice const> devices,
            std::span<VkCommandPoolCreateFlags const> flags,
            std::span<std::uint32_t> const queue_family_indices,
            std::pmr::polymorphic_allocator<> const& allocator
        ) noexcept;
        Command_pools_resources(Command_pools_resources const&) = delete;
        Command_pools_resources(Command_pools_resources&& other) noexcept;
        ~Command_pools_resources() noexcept;

        Command_pools_resources& operator=(Command_pools_resources const&) = delete;
        Command_pools_resources& operator=(Command_pools_resources&& other) noexcept;

        std::pmr::vector<VkDevice> devices;
        std::pmr::vector<VkCommandPool> command_pools;
    };

    export std::pmr::vector<std::pmr::vector<VkCommandBuffer>> allocate_command_buffers(
        std::span<VkDevice const> devices,
        std::span<VkCommandPool const> command_pools,
        VkCommandBufferLevel level,
        std::uint32_t number_of_frames_in_flight,
        VkAllocationCallbacks const* vulkan_allocator,
        std::pmr::polymorphic_allocator<> const& pmr_allocator
    );
}
