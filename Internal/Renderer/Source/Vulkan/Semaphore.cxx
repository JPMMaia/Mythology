export module maia.renderer.vulkan.semaphore;

import maia.renderer.vulkan.allocation_callbacks;
import maia.renderer.vulkan.device;

import <vulkan/vulkan.h>;

import <optional>;
import <span>;

namespace Maia::Renderer::Vulkan
{
    export struct Semaphore
    {
        VkSemaphore value;
    };

    export Semaphore create_semaphore(
        Device device,
        VkSemaphoreCreateFlags flags,
        std::optional<Allocation_callbacks> allocator
    ) noexcept;

    export void destroy_semaphore(
        Device device,
        Semaphore semaphore,
        std::optional<Allocation_callbacks> allocator
    ) noexcept;
}
