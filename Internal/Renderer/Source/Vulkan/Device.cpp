module maia.renderer.vulkan.device;

import maia.renderer.vulkan.check;
import maia.renderer.vulkan.physical_device;

import <vulkan/vulkan.h>;

import <cassert>;
import <cstdint>;
import <memory_resource>;
import <span>;
import <type_traits>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    std::uint32_t get_physical_device_queue_family_count(Physical_device const physical_device) noexcept
    {
        std::uint32_t queue_family_property_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device.value, &queue_family_property_count, nullptr);
        return queue_family_property_count;
    }

    std::pmr::vector<Queue_family_properties> get_physical_device_queue_family_properties(Physical_device const physical_device, std::pmr::polymorphic_allocator<Physical_device> const& allocator) noexcept
    {
        std::uint32_t queue_family_property_count = get_physical_device_queue_family_count(physical_device);

        std::pmr::vector<Queue_family_properties> queue_family_propertiess{queue_family_property_count, allocator};

        static_assert(std::is_standard_layout_v<Queue_family_properties>, "Must be standard layout so that Queue_family_properties and Queue_family_properties.value are pointer-interconvertible");
        static_assert(sizeof(Queue_family_properties) == sizeof(VkQueueFamilyProperties), "Queue_family_properties must only contain VkQueueFamilyProperties since using Queue_family_properties* as a contiguous array");
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device.value, &queue_family_property_count, reinterpret_cast<VkQueueFamilyProperties*>(queue_family_propertiess.data()));

        return queue_family_propertiess;
    }

    bool has_graphics_capabilities(Queue_family_properties const& queue_family_properties) noexcept
    {
        return queue_family_properties.value.queueFlags | VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT;
    }
    bool has_compute_capabilities(Queue_family_properties const& queue_family_properties) noexcept
    {
        return queue_family_properties.value.queueFlags | VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT;
    }
    bool has_transfer_capabilities(Queue_family_properties const& queue_family_properties) noexcept
    {
        return queue_family_properties.value.queueFlags | VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT;
    }


    Device_queue_create_info create_device_queue_create_info(std::uint32_t const queue_family_index, std::uint32_t const queue_count, std::span<float const> const queue_priorities) noexcept
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


    Device create_device(Physical_device const physical_device, std::span<Device_queue_create_info const> const queue_create_infos, std::span<char const* const> const enabled_extensions) noexcept
    {
        static_assert(std::is_standard_layout_v<Device_queue_create_info>, "Must be standard layout so that Device_queue_create_info and Device_queue_create_info.value are pointer-interconvertible");
        static_assert(sizeof(Device_queue_create_info) == sizeof(VkDeviceQueueCreateInfo), "Device_queue_create_info must only contain VkDeviceQueueCreateInfo since using Device_queue_create_info* as a contiguous array");

        VkDeviceCreateInfo const create_info
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueCreateInfoCount = static_cast<std::uint32_t>(queue_create_infos.size()),
            .pQueueCreateInfos = reinterpret_cast<VkDeviceQueueCreateInfo const*>(queue_create_infos.data()),
            .enabledExtensionCount = static_cast<std::uint32_t>(enabled_extensions.size()),
            .ppEnabledExtensionNames = enabled_extensions.data(),
            .pEnabledFeatures = nullptr
        };

        VkDevice device = {};
        check_result(
            vkCreateDevice(physical_device.value, &create_info, nullptr, &device));

        return {device};
    }
}