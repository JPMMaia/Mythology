module maia.renderer.vulkan.fence;

import maia.renderer.vulkan.allocation_callbacks;
import maia.renderer.vulkan.check;
import maia.renderer.vulkan.device;

import <vulkan/vulkan.h>;

import <memory_resource>;
import <optional>;
import <span>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    Fence create_fence(
        Device const device,
        VkFenceCreateFlags const flags,
        std::optional<Allocation_callbacks> const allocator
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
                device.value,
                &create_info,
                allocator.has_value() ? &allocator->value : nullptr,
                &fence
            )
        );

        return {fence};
    }

    std::pmr::vector<Fence> create_fences(
        std::size_t const count,
        Device const device,
        VkFenceCreateFlags const flags,
        std::optional<Allocation_callbacks> const vulkan_allocator,
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
        Device const device,
        Fence const fence,
        std::optional<Allocation_callbacks> const allocator
    ) noexcept
    {
        vkDestroyFence(
            device.value,
            fence.value,
            allocator.has_value() ? &allocator->value : nullptr
        );
    }

    bool is_fence_signaled(
        Device const device,
        Fence const fence
    ) noexcept
    {
        VkResult const state = vkGetFenceStatus(device.value, fence.value);

        if (state == VK_ERROR_DEVICE_LOST)
        {
            check_result(state);
        }
        
        return state == VK_SUCCESS;
    }

    void reset_fences(
        Device const device,
        std::span<Fence const> const fences
    ) noexcept
    {
        static_assert(std::is_standard_layout_v<Fence>, "Must be standard layout so that Fence and Fence.value are pointer-interconvertible");
        static_assert(sizeof(Fence) == sizeof(VkFence), "Fence must only contain VkFence since using Fence* as a contiguous array");

        check_result(
            vkResetFences(
                device.value,
                static_cast<uint32_t>(fences.size()),
                reinterpret_cast<VkFence const*>(fences.data())
            )
        );
    }

    VkResult wait_for_all_fences(
        Device const device,
        std::span<Fence const> const fences,
        Timeout_nanoseconds const timeout
    ) noexcept
    {
        static_assert(std::is_standard_layout_v<Fence>, "Must be standard layout so that Fence and Fence.value are pointer-interconvertible");
        static_assert(sizeof(Fence) == sizeof(VkFence), "Fence must only contain VkFence since using Fence* as a contiguous array");

        VkResult const result = 
            vkWaitForFences(
                device.value, 
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
        Device const device,
        std::span<Fence const> const fences,
        Timeout_nanoseconds const timeout
    ) noexcept
    {
        static_assert(std::is_standard_layout_v<Fence>, "Must be standard layout so that Fence and Fence.value are pointer-interconvertible");
        static_assert(sizeof(Fence) == sizeof(VkFence), "Fence must only contain VkFence since using Fence* as a contiguous array");

        VkResult const result = 
            vkWaitForFences(
                device.value, 
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
