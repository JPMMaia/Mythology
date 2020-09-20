export module maia.renderer.vulkan.fence;


import <vulkan/vulkan.h>;

import <memory_resource>;
import <optional>;
import <span>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    export VkFence create_fence(
        VkDevice device,
        VkFenceCreateFlags flags,
        VkAllocationCallbacks const* allocator = {}
    ) noexcept;

    export std::pmr::vector<VkFence> create_fences(
        std::size_t count,
        VkDevice device,
        VkFenceCreateFlags flags,
        VkAllocationCallbacks const* vulkan_allocator = {},
        std::pmr::polymorphic_allocator<VkFence> vector_allocator = {}
    );

    export void destroy_fence(
        VkDevice device,
        VkFence fence,
        VkAllocationCallbacks const* allocator
    ) noexcept;

    export bool is_fence_signaled(
        VkDevice device,
        VkFence fence
    ) noexcept;

    export void reset_fences(
        VkDevice device,
        std::span<VkFence const> fences
    ) noexcept;

    export struct Timeout_nanoseconds
    {
        std::uint64_t value;
    };

    export VkResult wait_for_all_fences(
        VkDevice device,
        std::span<VkFence const> fences,
        Timeout_nanoseconds timeout
    ) noexcept;

    export VkResult wait_for_any_fence(
        VkDevice device,
        std::span<VkFence const> fences,
        Timeout_nanoseconds timeout
    ) noexcept;

    export struct Wait_for_all_fences_lock
    {
        Wait_for_all_fences_lock(VkDevice const device, std::span<VkFence const> const fences, Timeout_nanoseconds const timeout) noexcept :
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
        std::span<VkFence const> fences;
        Timeout_nanoseconds timeout;
    };
}
