export module maia.renderer.vulkan.fence;


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
        VkDevice device,
        VkFenceCreateFlags flags,
        VkAllocationCallbacks const* allocator = {}
    ) noexcept;

    export std::pmr::vector<Fence> create_fences(
        std::size_t count,
        VkDevice device,
        VkFenceCreateFlags flags,
        VkAllocationCallbacks const* vulkan_allocator = {},
        std::pmr::polymorphic_allocator<Fence> vector_allocator = {}
    );

    export void destroy_fence(
        VkDevice device,
        Fence fence,
        VkAllocationCallbacks const* allocator
    ) noexcept;

    export bool is_fence_signaled(
        VkDevice device,
        Fence fence
    ) noexcept;

    export void reset_fences(
        VkDevice device,
        std::span<Fence const> fences
    ) noexcept;

    export struct Timeout_nanoseconds
    {
        std::uint64_t value;
    };

    export VkResult wait_for_all_fences(
        VkDevice device,
        std::span<Fence const> fences,
        Timeout_nanoseconds timeout
    ) noexcept;

    export VkResult wait_for_any_fence(
        VkDevice device,
        std::span<Fence const> fences,
        Timeout_nanoseconds timeout
    ) noexcept;

    export struct Wait_for_all_fences_lock
    {
        Wait_for_all_fences_lock(VkDevice const device, std::span<Fence const> const fences, Timeout_nanoseconds const timeout) noexcept :
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

        VkDevice device;
        std::span<Fence const> fences;
        Timeout_nanoseconds timeout;
    };
}
