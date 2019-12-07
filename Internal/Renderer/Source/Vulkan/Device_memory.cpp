module maia.renderer.vulkan.device_memory;

import maia.renderer.vulkan.check;
import maia.renderer.vulkan.allocation_callbacks;
import maia.renderer.vulkan.device;
import maia.renderer.vulkan.physical_device;

import <vulkan/vulkan.h>;

import <cstdint>;
import <memory_resource>;
import <ostream>;
import <sstream>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    namespace 
    {
        std::string to_string(VkMemoryPropertyFlagBits const flag) noexcept
        {
            switch (flag)
            {
            case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT:
                return "DEVICE_LOCAL";
            case VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT:
                return "HOST_VISIBLE";
            case VK_MEMORY_PROPERTY_HOST_COHERENT_BIT:
                return "HOST_COHERENT";
            case VK_MEMORY_PROPERTY_HOST_CACHED_BIT:
                return "HOST_CACHED";
            case VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT:
                return "LAZILY_ALLOCATED";
            case VK_MEMORY_PROPERTY_PROTECTED_BIT:
                return "PROTECTED";
            case VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD:
                return "DEVICE_COHERENT";
            case VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD:
                return "DEVICE_UNCACHED";
            default:
                return "UNKNOWN";
            }
        }

        std::string to_string(VkMemoryHeapFlagBits const flag) noexcept
        {
            switch (flag)
            {
            case VK_MEMORY_HEAP_DEVICE_LOCAL_BIT:
                return "DEVICE_LOCAL";
            case VK_MEMORY_HEAP_MULTI_INSTANCE_BIT:
                return "MULTI_INSTANCE";
            default:
                return "UNKNOWN";
            }
        }

        template <class Flag_bits, class Flags, class Cast_type = std::uint32_t>
        std::string to_string(Flags const flags) noexcept
        {
            std::pmr::vector<Flag_bits> const flag_bits = [flags]() -> std::pmr::vector<Flag_bits> 
            {
                std::pmr::vector<Flag_bits> flag_bits;

                for (std::uint8_t bit_index = 0; bit_index < 32; ++bit_index)
                {
                    Cast_type const mask = 1 << bit_index;
                    Cast_type const result = static_cast<Cast_type>(flags) & mask;

                    if (result != 0)
                    {
                        flag_bits.push_back(static_cast<Flag_bits>(result));
                    }
                }

                return flag_bits;
            }();

            std::stringstream ss;

            for (std::size_t i = 1; i < flag_bits.size(); ++i)
            {
                ss << to_string(flag_bits[i - 1]) << " | ";
            }
            ss << to_string(flag_bits.back());

            return ss.str();
        }
    }

    std::ostream& operator<<(std::ostream& output_stream, VkMemoryType const& memory_type) noexcept
    {
        output_stream << "Memory type:\n";
        output_stream << "\tProperty flags: " << to_string<VkMemoryPropertyFlagBits>(memory_type.propertyFlags) << '\n';
        output_stream << "\tHeap index: " << memory_type.heapIndex << '\n';

        return output_stream;
    }

    std::ostream& operator<<(std::ostream& output_stream, VkMemoryHeap const& memory_heap) noexcept
    {
        output_stream << "Memory heap:\n";
        output_stream << "\tSize: " << memory_heap.size;
        output_stream << "\tFlags: " << to_string<VkMemoryHeapFlagBits>(memory_heap.flags);

        return output_stream;
    }

    std::ostream& operator<<(std::ostream& output_stream, Physical_device_memory_properties const& physical_device_memory_properties) noexcept
    {
        std::for_each(
            physical_device_memory_properties.value.memoryTypes, 
            physical_device_memory_properties.value.memoryTypes + physical_device_memory_properties.value.memoryTypeCount,
            [&output_stream](VkMemoryType const& memory_type) -> void { output_stream << memory_type << '\n'; }
        );

        std::for_each(
            physical_device_memory_properties.value.memoryHeaps, 
            physical_device_memory_properties.value.memoryHeaps + physical_device_memory_properties.value.memoryHeapCount,
            [&output_stream](VkMemoryHeap const& memory_heap) -> void { output_stream << memory_heap << '\n'; }
        );

        return output_stream;
    }


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