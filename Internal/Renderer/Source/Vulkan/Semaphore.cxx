export module maia.renderer.vulkan.semaphore;

import <vulkan/vulkan.h>;

import <memory_resource>;
import <optional>;
import <span>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    export struct Semaphore_value
    {
        std::uint64_t value = 0;
    };

    export VkSemaphore create_semaphore(
        VkDevice device,
        VkSemaphoreType semaphore_type,
        Semaphore_value initial_value = {},
        VkSemaphoreCreateFlags flags = {},
        VkAllocationCallbacks const* allocator = {}
    ) noexcept;

    export std::pmr::vector<VkSemaphore> create_semaphores(
        std::size_t count,
        VkDevice device,
        VkSemaphoreType semaphore_type,
        Semaphore_value initial_value = {},
        VkSemaphoreCreateFlags flags = {},
        VkAllocationCallbacks const* vulkan_allocator = {},
        std::pmr::polymorphic_allocator<VkSemaphore> vector_allocator = {}
    ) noexcept;

    export void destroy_semaphore(
        VkDevice device,
        VkSemaphore semaphore,
        VkAllocationCallbacks const* allocator
    ) noexcept;
}
