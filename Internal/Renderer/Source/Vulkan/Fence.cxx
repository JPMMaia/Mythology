export module maia.renderer.vulkan.fence;

import maia.renderer.vulkan.allocation_callbacks;
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
        std::optional<Allocation_callbacks> allocator = {}
    ) noexcept;

    export std::pmr::vector<Fence> create_fences(
        std::size_t count,
        Device device,
        VkFenceCreateFlags flags,
        std::optional<Allocation_callbacks> vulkan_allocator = {},
        std::pmr::polymorphic_allocator<Fence> vector_allocator = {}
    );

    export void destroy_fence(
        Device device,
        Fence fence,
        std::optional<Allocation_callbacks> allocator
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
}
