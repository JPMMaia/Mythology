export module maia.renderer.vulkan.fence;

import maia.renderer.vulkan.device;

import <vulkan/vulkan.h>;

import <memory_resource>;
import <optional>;
import <span>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    export struct Fence
    {
        VkFence value;
    };

    export Fence create_fence(
        Device device,
        VkFenceCreateFlags flags,
        VkAllocationCallbacks const* allocator = {}
    ) noexcept;

    export std::pmr::vector<Fence> create_fences(
        std::size_t count,
        Device device,
        VkFenceCreateFlags flags,
        VkAllocationCallbacks const* vulkan_allocator = {},
        std::pmr::polymorphic_allocator<Fence> vector_allocator = {}
    );

    export void destroy_fence(
        Device device,
        Fence fence,
        VkAllocationCallbacks const* allocator
    ) noexcept;

    export bool is_fence_signaled(
        Device device,
        Fence fence
    ) noexcept;

    export void reset_fences(
        Device device,
        std::span<Fence const> fences
    ) noexcept;

    export struct Timeout_nanoseconds
    {
        std::uint64_t value;
    };

    export VkResult wait_for_all_fences(
        Device device,
        std::span<Fence const> fences,
        Timeout_nanoseconds timeout
    ) noexcept;

    export VkResult wait_for_any_fence(
        Device device,
        std::span<Fence const> fences,
        Timeout_nanoseconds timeout
    ) noexcept;

    export struct Wait_for_all_fences_lock
    {
        Wait_for_all_fences_lock(Device const device, std::span<Fence const> const fences, Timeout_nanoseconds const timeout) noexcept :
            device{device},
            fences{fences},
            timeout{timeout}
        {
        }
        Wait_for_all_fences_lock(Wait_for_all_fences_lock const&) noexcept = delete;
        Wait_for_all_fences_lock(Wait_for_all_fences_lock&&) noexcept = delete;
        ~Wait_for_all_fences_lock() noexcept
        {
            wait_for_all_fences(this->device, this->fences, this->timeout);
        }

        Wait_for_all_fences_lock& operator=(Wait_for_all_fences_lock const&) noexcept = delete;
        Wait_for_all_fences_lock& operator=(Wait_for_all_fences_lock&&) noexcept = delete;

        Device device;
        std::span<Fence const> fences;
        Timeout_nanoseconds timeout;
    };
}
