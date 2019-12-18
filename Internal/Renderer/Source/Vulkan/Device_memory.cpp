module maia.renderer.vulkan.device_memory;

import maia.renderer.vulkan.allocation_callbacks;
import maia.renderer.vulkan.buffer;
import maia.renderer.vulkan.check;
import maia.renderer.vulkan.device;
import maia.renderer.vulkan.image;
import maia.renderer.vulkan.physical_device;

import <vulkan/vulkan.h>;

import <cstdint>;
import <memory_resource>;
import <optional>;
import <ostream>;
import <sstream>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    Memory_requirements get_memory_requirements(
        Device const device,
        Buffer const buffer
    ) noexcept
    {
        VkMemoryRequirements memory_requirements = {};
        vkGetBufferMemoryRequirements(device.value, buffer.value, &memory_requirements);
        return {memory_requirements};
    }

    Memory_requirements get_memory_requirements(
        Device const device,
        Image const image
    ) noexcept
    {
        VkMemoryRequirements memory_requirements = {};
        vkGetImageMemoryRequirements(device.value, image.value, &memory_requirements);
        return {memory_requirements};
    }


    Memory_type_bits get_memory_type_bits(
        Memory_requirements memory_requirements
    ) noexcept
    {
        return {memory_requirements.value.memoryTypeBits};
    }


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


    std::optional<Memory_type_index> find_memory_type(
        Physical_device_memory_properties const& memory_properties,
        Memory_type_bits const memory_type_bits_requirement,
        VkMemoryPropertyFlags const required_properties
    ) noexcept
    {
        std::uint32_t const memory_count = memory_properties.value.memoryTypeCount;

        for (std::uint32_t memory_index = 0; memory_index < memory_count; ++memory_index)
        {
            std::uint32_t const memory_type_bits = (1 << memory_index);
            bool const is_required_memory_type = memory_type_bits_requirement.value & memory_type_bits;

            VkMemoryPropertyFlags const properties =
                memory_properties.value.memoryTypes[memory_index].propertyFlags;
            bool const has_required_properties =
                (properties & required_properties) == required_properties;

            if (is_required_memory_type && has_required_properties)
                return {{memory_index}};
        }

        return {};
    }

    std::optional<Memory_type_index> find_memory_type(
        Physical_device_memory_properties const& memory_properties,
        Memory_type_bits const memory_type_bits_requirement,
        VkMemoryPropertyFlags const required_properties,
        VkMemoryPropertyFlags const optimal_properties
    ) noexcept
    {
        std::optional<Memory_type_index> const memory_type_index =
            find_memory_type(memory_properties, memory_type_bits_requirement, optimal_properties);

        if (memory_type_index.has_value())
        {
            return memory_type_index;
        }
        else 
        {
            return find_memory_type(memory_properties, memory_type_bits_requirement, required_properties);
        }
    }


    Device_memory allocate_memory(
        Device const device, 
        VkDeviceSize const allocation_size, 
        Memory_type_index const memory_type_index, 
        std::optional<Allocation_callbacks> const allocator
    ) noexcept
    {
        VkMemoryAllocateInfo const info
        {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = nullptr,
            .allocationSize = allocation_size,
            .memoryTypeIndex = memory_type_index.value
        };

        VkDeviceMemory memory = {};
        check_result(
            vkAllocateMemory(
                device.value, 
                &info, 
                allocator.has_value() ? &allocator->value : nullptr,
                &memory
            )
        );

        return {memory};
    }

    void bind_memory(
        Device const device,
        Buffer const buffer,
        Device_memory const memory,
        VkDeviceSize const memory_offset
    ) noexcept
    {
        check_result(
            vkBindBufferMemory(device.value, buffer.value, memory.value, memory_offset));
    }

    void bind_memory(
        Device const device,
        Image const image,
        Device_memory const memory,
        VkDeviceSize const memory_offset
    ) noexcept
    {
        check_result(
            vkBindImageMemory(device.value, image.value, memory.value, memory_offset));
    }


    void* map_memory(
        Device const device,
        Device_memory const device_memory,
        VkDeviceSize const offset,
        VkDeviceSize const size,
        VkMemoryMapFlags const flags
    ) noexcept
    {
        void* data = nullptr;
        check_result(
            vkMapMemory(
                device.value,
                device_memory.value,
                offset,
                size,
                flags,
                &data
            )
        );
        return data;
    }

    void unmap_memory(
        Device const device,
        Device_memory const device_memory
    ) noexcept
    {
        vkUnmapMemory(
            device.value,
            device_memory.value
        );
    }
}