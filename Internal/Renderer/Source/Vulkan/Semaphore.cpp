module maia.renderer.vulkan.semaphore;

import maia.renderer.vulkan.allocation_callbacks;
import maia.renderer.vulkan.check;
import maia.renderer.vulkan.device;

import <vulkan/vulkan.h>;

import <optional>;
import <span>;

namespace Maia::Renderer::Vulkan
{
    Semaphore create_semaphore(
        Device const device,
        VkSemaphoreType const semaphore_type,
        Semaphore_value const initial_value,
        VkSemaphoreCreateFlags const flags,
        std::optional<Allocation_callbacks> const allocator
    ) noexcept
    {
        VkSemaphoreTypeCreateInfo const type_create_info
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
            .pNext = nullptr,
            .semaphoreType = semaphore_type,
            .initialValue = initial_value.value
        };

        VkSemaphoreCreateInfo const create_info
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = &type_create_info,
            .flags = flags
        };

        VkSemaphore semaphore;
        check_result(
            vkCreateSemaphore(
                device.value, 
                &create_info,
                allocator.has_value() ? &allocator->value : nullptr,
                &semaphore
            )
        );

        return {semaphore};
    }

    void destroy_semaphore(
        Device const device,
        Semaphore const semaphore,
        std::optional<Allocation_callbacks> const allocator
    ) noexcept
    {
        vkDestroySemaphore(
            device.value,
            semaphore.value,
            allocator.has_value() ? &allocator->value : nullptr
        );
    }
}
