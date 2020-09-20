module maia.renderer.vulkan.device;

import maia.renderer.vulkan.check;

import <vulkan/vulkan.h>;

import <cassert>;
import <cstdint>;
import <memory_resource>;
import <optional>;
import <span>;
import <type_traits>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    std::uint32_t get_physical_device_queue_family_count(VkPhysicalDevice const physical_device) noexcept
    {
        std::uint32_t queue_family_property_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_property_count, nullptr);
        return queue_family_property_count;
    }

    std::pmr::vector<VkQueueFamilyProperties> get_physical_device_queue_family_properties(VkPhysicalDevice const physical_device, std::pmr::polymorphic_allocator<VkPhysicalDevice> const& allocator) noexcept
    {
        std::uint32_t queue_family_property_count = get_physical_device_queue_family_count(physical_device);

        std::pmr::vector<VkQueueFamilyProperties> queue_family_propertiess{queue_family_property_count, allocator};
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_property_count, queue_family_propertiess.data());

        return queue_family_propertiess;
    }

    bool has_graphics_capabilities(VkQueueFamilyProperties const& queue_family_properties) noexcept
    {
        return queue_family_properties.value.queueFlags | VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT;
    }
    bool has_compute_capabilities(VkQueueFamilyProperties const& queue_family_properties) noexcept
    {
        return queue_family_properties.value.queueFlags | VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT;
    }
    bool has_transfer_capabilities(VkQueueFamilyProperties const& queue_family_properties) noexcept
    {
        return queue_family_properties.value.queueFlags | VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT;
    }

    bool operator==(Queue_family_index const lhs, Queue_family_index const rhs) noexcept
    {
        return lhs.value == rhs.value;
    }
    bool operator!=(Queue_family_index const lhs, Queue_family_index const rhs) noexcept
    {
        return !(lhs == rhs);
    }


    VkDeviceQueueCreateInfo create_device_queue_create_info(std::uint32_t const queue_family_index, std::uint32_t const queue_count, std::span<float const> const queue_priorities) noexcept
    {
        assert(queue_count == queue_priorities.size());

        VkDeviceQueueCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;
        create_info.queueFamilyIndex = queue_family_index;
        create_info.queueCount = queue_count;
        create_info.pQueuePriorities = queue_priorities.data();
        return { create_info };
    }


    VkDevice create_device(VkPhysicalDevice const physical_device, std::span<VkDeviceQueueCreateInfo const> const queue_create_infos, std::span<char const* const> const enabled_extensions) noexcept
    {
        VkDeviceCreateInfo const create_info
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueCreateInfoCount = static_cast<std::uint32_t>(queue_create_infos.size()),
            .pQueueCreateInfos = queue_create_infos.data(),
            .enabledExtensionCount = static_cast<std::uint32_t>(enabled_extensions.size()),
            .ppEnabledExtensionNames = enabled_extensions.data(),
            .pEnabledFeatures = nullptr
        };

        VkDevice device = VK_NULL_HANDLE;
        check_result(
            vkCreateDevice(physical_device, &create_info, nullptr, &device));

        return {device};
    }

    void destroy_device(VkDevice const device, VkAllocationCallbacks const* const allocator) noexcept
    {
        vkDestroyDevice(device, allocator);
    }
}