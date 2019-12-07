module maia.renderer.vulkan.device_memory;

import maia.renderer.vulkan.check;
import maia.renderer.vulkan.allocation_callbacks;
import maia.renderer.vulkan.device;
import maia.renderer.vulkan.physical_device;

import <vulkan/vulkan.h>;

import <cstdint>;

namespace Maia::Renderer::Vulkan
{
    struct Physical_device_memory_properties
    {
        VkPhysicalDeviceMemoryProperties value;
    };

    Physical_device_memory_properties get_phisical_device_memory_properties(Physical_device const physical_device) noexcept
    {
        Physical_device_memory_properties memory_properties = {};
        vkGetPhysicalDeviceMemoryProperties(physical_device.value, &memory_properties.value);
        return memory_properties;
    }

    Device_memory allocate_memory(Device const device, VkDeviceSize const allocation_size, std::uint32_t const memory_type_index, Allocation_callbacks const allocator) noexcept
    {
        VkMemoryAllocateInfo const info
        {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = nullptr,
            .allocationSize = allocation_size,
            .memoryTypeIndex = memory_type_index
        };

        VkDeviceMemory memory = {};
        check_result(
            vkAllocateMemory(device.value, &info, &allocator.value, &memory));

        return {memory};
    }
}