module maia.renderer.vulkan.fence;

import maia.renderer.vulkan.check;

import <vulkan/vulkan.h>;

import <memory_resource>;
import <optional>;
import <span>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    Fence create_fence(
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

    std::pmr::vector<Fence> create_fences(
        std::size_t const count,
        VkDevice const device,
        VkFenceCreateFlags const flags,
        VkAllocationCallbacks const* const vulkan_allocator,
        std::pmr::polymorphic_allocator<Fence> vector_allocator
    )
    {
        std::pmr::vector<Fence> fences{std::move(vector_allocator)};
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
        Fence const fence,
        VkAllocationCallbacks const* const allocator
    ) noexcept
    {
        vkDestroyFence(
            device,
            fence.value,
            allocator
        );
    }

    bool is_fence_signaled(
        VkDevice const device,
        Fence const fence
    ) noexcept
    {
        VkResult const state = vkGetFenceStatus(device, fence.value);

        if (state == VK_ERROR_DEVICE_LOST)
        {
            check_result(state);
        }
        
        return state == VK_SUCCESS;
    }

    void reset_fences(
        VkDevice const device,
        std::span<Fence const> const fences
    ) noexcept
    {
        static_assert(std::is_standard_layout_v<Fence>, "Must be standard layout so that Fence and Fence.value are pointer-interconvertible");
        static_assert(sizeof(Fence) == sizeof(VkFence), "Fence must only contain VkFence since using Fence* as a contiguous array");

        check_result(
            vkResetFences(
                device,
                static_cast<uint32_t>(fences.size()),
                reinterpret_cast<VkFence const*>(fences.data())
            )
        );
    }

    VkResult wait_for_all_fences(
        VkDevice const device,
        std::span<Fence const> const fences,
        Timeout_nanoseconds const timeout
    ) noexcept
    {
        static_assert(std::is_standard_layout_v<Fence>, "Must be standard layout so that Fence and Fence.value are pointer-interconvertible");
        static_assert(sizeof(Fence) == sizeof(VkFence), "Fence must only contain VkFence since using Fence* as a contiguous array");

        VkResult const result = 
            vkWaitForFences(
                device, 
                static_cast<uint32_t>(fences.size()),
                reinterpret_cast<VkFence const*>(fences.data()),
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
        std::span<Fence const> const fences,
        Timeout_nanoseconds const timeout
    ) noexcept
    {
        static_assert(std::is_standard_layout_v<Fence>, "Must be standard layout so that Fence and Fence.value are pointer-interconvertible");
        static_assert(sizeof(Fence) == sizeof(VkFence), "Fence must only contain VkFence since using Fence* as a contiguous array");

        VkResult const result = 
            vkWaitForFences(
                device, 
                static_cast<uint32_t>(fences.size()),
                reinterpret_cast<VkFence const*>(fences.data()),
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
