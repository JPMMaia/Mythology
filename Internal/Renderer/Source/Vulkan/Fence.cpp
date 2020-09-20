module maia.renderer.vulkan.fence;

import maia.renderer.vulkan.check;

import <vulkan/vulkan.h>;

import <memory_resource>;
import <optional>;
import <span>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    VkFence create_fence(
        VkDevice const device,
        VkFenceCreateFlags const flags,
        VkAllocationCallbacks const* const allocator
    ) noexcept
    {
        VkFenceCreateInfo const create_info
        {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = flags
        };

        VkFence fence = {};
        check_result(
            vkCreateFence(
                device,
                &create_info,
                allocator,
                &fence
            )
        );

        return {fence};
    }

    std::pmr::vector<VkFence> create_fences(
        std::size_t const count,
        VkDevice const device,
        VkFenceCreateFlags const flags,
        VkAllocationCallbacks const* const vulkan_allocator,
        std::pmr::polymorphic_allocator<VkFence> vector_allocator
    )
    {
        std::pmr::vector<VkFence> fences{std::move(vector_allocator)};
        fences.resize(count);

        for (std::size_t index = 0; index < count; ++index)
        {
            fences[index] = 
                create_fence(device, flags, vulkan_allocator);
        }

        return fences;
    }

    void destroy_fence(
        VkDevice const device,
        VkFence const fence,
        VkAllocationCallbacks const* const allocator
    ) noexcept
    {
        vkDestroyFence(
            device,
            fence,
            allocator
        );
    }

    bool is_fence_signaled(
        VkDevice const device,
        VkFence const fence
    ) noexcept
    {
        VkResult const state = vkGetFenceStatus(device, fence);

        if (state == VK_ERROR_DEVICE_LOST)
        {
            check_result(state);
        }
        
        return state == VK_SUCCESS;
    }

    void reset_fences(
        VkDevice const device,
        std::span<VkFence const> const fences
    ) noexcept
    {
        check_result(
            vkResetFences(
                device,
                static_cast<uint32_t>(fences.size()),
                fences.data()
            )
        );
    }

    VkResult wait_for_all_fences(
        VkDevice const device,
        std::span<VkFence const> const fences,
        Timeout_nanoseconds const timeout
    ) noexcept
    {
        VkResult const result = 
            vkWaitForFences(
                device, 
                static_cast<uint32_t>(fences.size()),
                fences.data(),
                VK_TRUE,
                timeout.value
            );

        if (result == VK_SUCCESS || result == VK_TIMEOUT)
        {
            return result;
        }
        else
        {
            check_result(result);
            return result;
        }
    }

    VkResult wait_for_any_fence(
        VkDevice const device,
        std::span<VkFence const> const fences,
        Timeout_nanoseconds const timeout
    ) noexcept
    {
        VkResult const result = 
            vkWaitForFences(
                device, 
                static_cast<uint32_t>(fences.size()),
                fences.data(),
                VK_FALSE,
                timeout.value
            );

        if (result == VK_SUCCESS || result == VK_TIMEOUT)
        {
            return result;
        }
        else
        {
            check_result(result);
            return result;
        }
    }
}
