module;

#include <vulkan/vulkan.hpp>

#include <memory_resource>
#include <optional>
#include <span>
#include <vector>

export module mythology.sdl.render_resources;

namespace Mythology::Render
{
    export struct Instance_resources
    {
        Instance_resources(
            std::uint32_t api_version,
            std::span<char const* const> required_instance_extensions
        );
        Instance_resources(Instance_resources const&) = delete;
        Instance_resources(Instance_resources&& other) noexcept;
        ~Instance_resources() noexcept;

        Instance_resources& operator=(Instance_resources const&) = delete;
        Instance_resources& operator=(Instance_resources&& other) noexcept;

        vk::Instance instance = VK_NULL_HANDLE;
    };

    export struct Frame_synchronization_resources
    {
        std::pmr::vector<vk::Semaphore> available_frame_semaphores;
        std::pmr::vector<vk::Semaphore> finished_frame_semaphores;
        std::pmr::vector<vk::Fence> available_frame_fences;
    };

    export struct Synchronization_resources
    {
        Synchronization_resources(
            std::size_t number_of_frames,
            std::span<vk::Device const> devices,
            std::pmr::polymorphic_allocator<> const& allocator
        );
        Synchronization_resources(Synchronization_resources const&) = delete;
        Synchronization_resources(Synchronization_resources&& other) noexcept;
        ~Synchronization_resources() noexcept;

        Synchronization_resources& operator=(Synchronization_resources const&) = delete;
        Synchronization_resources& operator=(Synchronization_resources&& other) noexcept;

        std::pmr::vector<vk::Device> devices;
        std::pmr::vector<Frame_synchronization_resources> frames;
    };

    export struct Command_pools_resources
    {
        Command_pools_resources(
            std::span<vk::Device const> devices,
            std::span<vk::CommandPoolCreateFlags const> flags,
            std::span<std::uint32_t> const queue_family_indices,
            std::pmr::polymorphic_allocator<> const& allocator
        ) noexcept;
        Command_pools_resources(Command_pools_resources const&) = delete;
        Command_pools_resources(Command_pools_resources&& other) noexcept;
        ~Command_pools_resources() noexcept;

        Command_pools_resources& operator=(Command_pools_resources const&) = delete;
        Command_pools_resources& operator=(Command_pools_resources&& other) noexcept;

        std::pmr::vector<vk::Device> devices;
        std::pmr::vector<vk::CommandPool> command_pools;
    };

    export std::pmr::vector<std::pmr::vector<vk::CommandBuffer>> allocate_command_buffers(
        std::span<vk::Device const> devices,
        std::span<vk::CommandPool const> command_pools,
        vk::CommandBufferLevel level,
        std::uint32_t number_of_frames_in_flight,
        vk::AllocationCallbacks const* vulkan_allocator,
        std::pmr::polymorphic_allocator<> const& pmr_allocator
    );
}
