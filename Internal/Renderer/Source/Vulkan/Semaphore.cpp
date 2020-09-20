module maia.renderer.vulkan.semaphore;

import maia.renderer.vulkan.check;

import <vulkan/vulkan.h>;

import <memory_resource>;
import <optional>;
import <span>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    VkSemaphore create_semaphore(
        VkDevice const device,
        VkSemaphoreType const semaphore_type,
        Semaphore_value const initial_value,
        VkSemaphoreCreateFlags const flags,
        VkAllocationCallbacks const* const allocator
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
                device, 
                &create_info,
                allocator,
                &semaphore
            )
        );

        return {semaphore};
    }

    std::pmr::vector<VkSemaphore> create_semaphores(
        std::size_t const count,
        VkDevice const device,
        VkSemaphoreType const semaphore_type,
        Semaphore_value const initial_value,
        VkSemaphoreCreateFlags const flags,
        VkAllocationCallbacks const* const vulkan_allocator,
        std::pmr::polymorphic_allocator<VkSemaphore> vector_allocator
    ) noexcept
    {
        std::pmr::vector<VkSemaphore> semaphores{std::move(vector_allocator)};
        semaphores.resize(count);

        for (std::size_t index = 0; index < count; ++index)
        {
            semaphores[index] = 
                create_semaphore(device, semaphore_type, initial_value, flags, vulkan_allocator);
        }

        return semaphores;
    }

    void destroy_semaphore(
        VkDevice const device,
        VkSemaphore const semaphore,
        VkAllocationCallbacks const* const allocator
    ) noexcept
    {
        vkDestroySemaphore(
            device,
            semaphore.value,
            allocator
        );
    }
}
