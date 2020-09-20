module maia.renderer.vulkan.device_memory;

import maia.renderer.vulkan.buffer;
import maia.renderer.vulkan.check;

import <vulkan/vulkan.h>;

import <algorithm>;
import <cstdint>;
import <memory_resource>;
import <optional>;
import <ostream>;
import <span>;
import <sstream>;
import <utility>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    VkMemoryRequirements get_memory_requirements(
        VkDevice const device,
        VkBuffer const buffer
    ) noexcept
    {
        VkMemoryRequirements memory_requirements = {};
        vkGetBufferMemoryRequirements(device, buffer, &memory_requirements);
        return {memory_requirements};
    }

    VkMemoryRequirements get_memory_requirements(
        VkDevice const device,
        VkImage const image
    ) noexcept
    {
        VkMemoryRequirements memory_requirements = {};
        vkGetImageMemoryRequirements(device, image, &memory_requirements);
        return {memory_requirements};
    }


    Memory_type_bits get_memory_type_bits(
        VkMemoryRequirements memory_requirements
    ) noexcept
    {
        return {memory_requirements.memoryTypeBits};
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
        /*
        output_stream << "Memory type:\n";
        output_stream << "\tProperty flags: " << to_string<VkMemoryPropertyFlagBits>(memory_type.propertyFlags) << '\n';
        output_stream << "\tHeap index: " << memory_type.heapIndex << '\n';
        */
        return output_stream;
    }

    std::ostream& operator<<(std::ostream& output_stream, VkMemoryHeap const& memory_heap) noexcept
    {
        /*
        output_stream << "Memory heap:\n";
        output_stream << "\tSize: " << memory_heap.size;
        output_stream << "\tFlags: " << to_string<VkMemoryHeapFlagBits>(memory_heap.flags);
        */
        return output_stream;
    }

    std::ostream& operator<<(std::ostream& output_stream, VkPhysicalDeviceMemoryProperties const& physical_device_memory_properties) noexcept
    {
        std::for_each(
            physical_device_memory_properties.memoryTypes, 
            physical_device_memory_properties.memoryTypes + physical_device_memory_properties.memoryTypeCount,
            [&output_stream](VkMemoryType const& memory_type) -> void { output_stream << memory_type << '\n'; }
        );

        std::for_each(
            physical_device_memory_properties.memoryHeaps, 
            physical_device_memory_properties.memoryHeaps + physical_device_memory_properties.memoryHeapCount,
            [&output_stream](VkMemoryHeap const& memory_heap) -> void { output_stream << memory_heap << '\n'; }
        );

        return output_stream;
    }


    VkPhysicalDeviceMemoryProperties get_phisical_device_memory_properties(VkPhysicalDevice const physical_device) noexcept
    {
        VkPhysicalDeviceMemoryProperties memory_properties = {};
        vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);
        return memory_properties;
    }


    std::optional<Memory_type_index_and_properties> find_memory_type(
        VkPhysicalDeviceMemoryProperties const& memory_properties,
        Memory_type_bits const memory_type_bits_requirement,
        VkMemoryPropertyFlags const required_properties
    ) noexcept
    {
        std::uint32_t const memory_count = memory_properties.memoryTypeCount;

        for (std::uint32_t memory_index = 0; memory_index < memory_count; ++memory_index)
        {
            std::uint32_t const memory_type_bits = (1 << memory_index);
            bool const is_required_memory_type = memory_type_bits_requirement.value & memory_type_bits;

            VkMemoryPropertyFlags const properties =
                memory_properties.memoryTypes[memory_index].propertyFlags;
            bool const has_required_properties =
                (properties & required_properties) == required_properties;

            if (is_required_memory_type && has_required_properties)
                return Memory_type_index_and_properties{{memory_index}, properties};
        }

        return {};
    }

    std::optional<Memory_type_index_and_properties> find_memory_type(
        VkPhysicalDeviceMemoryProperties const& memory_properties,
        Memory_type_bits const memory_type_bits_requirement,
        VkMemoryPropertyFlags const required_properties,
        VkMemoryPropertyFlags const optimal_properties
    ) noexcept
    {
        std::optional<Memory_type_index_and_properties> const memory_type_index_and_properties =
            find_memory_type(memory_properties, memory_type_bits_requirement, required_properties | optimal_properties);

        if (memory_type_index_and_properties.has_value())
        {
            return memory_type_index_and_properties;
        }
        else 
        {
            return find_memory_type(memory_properties, memory_type_bits_requirement, required_properties);
        }
    }


    VkDeviceMemory allocate_memory(
        VkDevice const device, 
        VkDeviceSize const allocation_size, 
        Memory_type_index const memory_type_index, 
        VkAllocationCallbacks const* const allocator
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
                device, 
                &info, 
                allocator,
                &memory
            )
        );

        return memory;
    }

    void free_memory(
        VkDevice const device,
        VkDeviceMemory const device_memory,
        VkAllocationCallbacks const* const allocator
    ) noexcept
    {
        vkFreeMemory(device, device_memory, allocator);
    }

    void bind_memory(
        VkDevice const device,
        VkBuffer const buffer,
        VkDeviceMemory const memory,
        VkDeviceSize const memory_offset
    ) noexcept
    {
        check_result(
            vkBindBufferMemory(device, buffer, memory, memory_offset));
    }

    void bind_memory(
        VkDevice const device,
        VkImage const image,
        VkDeviceMemory const memory,
        VkDeviceSize const memory_offset
    ) noexcept
    {
        check_result(
            vkBindImageMemory(device, image, memory, memory_offset));
    }

    void flush_mapped_memory_ranges(
        VkDevice const device,
        std::span<VkMappedMemoryRange const> const memory_ranges
    ) noexcept
    {
        check_result(
            vkFlushMappedMemoryRanges(device, static_cast<std::uint32_t>(memory_ranges.size()), memory_ranges.data()));
    }


    void* map_memory(
        VkDevice const device,
        VkDeviceMemory const device_memory,
        VkDeviceSize const offset,
        VkDeviceSize const size,
        VkMemoryMapFlags const flags
    ) noexcept
    {
        void* data = nullptr;
        check_result(
            vkMapMemory(
                device,
                device_memory,
                offset,
                size,
                flags,
                &data
            )
        );
        return data;
    }

    void unmap_memory(
        VkDevice const device,
        VkDeviceMemory const device_memory
    ) noexcept
    {
        vkUnmapMemory(
            device,
            device_memory
        );
    }


    Mapped_memory::Mapped_memory(
        VkDevice const device,
        VkDeviceMemory const device_memory,
        VkDeviceSize const offset,
        VkDeviceSize const size,
        VkMemoryMapFlags const flags
    ) noexcept :
        m_device{device},
        m_device_memory{device_memory},
        m_mapped_memory{map_memory(device, device_memory, offset, size, flags)}
    {
    }
    Mapped_memory::Mapped_memory(Mapped_memory&& other) noexcept :
        m_device{std::exchange(other.m_device, {})},
        m_device_memory{std::exchange(other.m_device_memory, {})},
        m_mapped_memory{std::exchange(other.m_mapped_memory, nullptr)}
    {
    }
    Mapped_memory::~Mapped_memory() noexcept
    {
        if (m_mapped_memory != nullptr)
        {
            unmap_memory(m_device, m_device_memory);
        }
    }

    Mapped_memory& Mapped_memory::operator=(Mapped_memory&& other) noexcept
    {
        m_device = std::exchange(other.m_device, {});
        m_device_memory = std::exchange(other.m_device_memory, {});
        m_mapped_memory = std::exchange(other.m_mapped_memory, nullptr);
        
        return *this;
    }

    void* Mapped_memory::data() const noexcept
    {
        return m_mapped_memory;
    }
}