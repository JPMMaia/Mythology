export module maia.renderer.vulkan.device_memory;

import maia.renderer.vulkan.allocation_callbacks;
import maia.renderer.vulkan.buffer;
import maia.renderer.vulkan.device;
import maia.renderer.vulkan.image;
import maia.renderer.vulkan.physical_device;

import <vulkan/vulkan.h>;

import <cstdint>;
import <optional>;
import <ostream>;

namespace Maia::Renderer::Vulkan
{
    export struct Memory_requirements
    {
        VkMemoryRequirements value;
    };

    export Memory_requirements get_memory_requirements(
        Device device,
        Buffer buffer
    ) noexcept;

    export Memory_requirements get_memory_requirements(
        Device device,
        Image image
    ) noexcept;


    export struct Memory_type_bits
    {
        std::uint32_t value;
    };

    export Memory_type_bits get_memory_type_bits(
        Memory_requirements memory_requirements
    ) noexcept;


    export struct Physical_device_memory_properties
    {
        VkPhysicalDeviceMemoryProperties value;
    };

    export std::ostream& operator<<(
        std::ostream& output_stream, 
        Physical_device_memory_properties const& physical_device_memory_properties
    ) noexcept;

    export Physical_device_memory_properties get_phisical_device_memory_properties(
        Physical_device const physical_device
    ) noexcept;


    export struct Memory_type_index
    {
        std::uint32_t value;
    };

    export std::optional<Memory_type_index> find_memory_type(
        Physical_device_memory_properties const& memory_properties,
        Memory_type_bits memory_type_bits_requirement,
        VkMemoryPropertyFlags required_properties
    ) noexcept;

    export std::optional<Memory_type_index> find_memory_type(
        Physical_device_memory_properties const& memory_properties,
        Memory_type_bits memory_type_bits_requirement,
        VkMemoryPropertyFlags required_properties,
        VkMemoryPropertyFlags optimal_properties
    ) noexcept;


    export struct Device_memory
    {
        VkDeviceMemory value;
    };

    export Device_memory allocate_memory(
        Device device, 
        VkDeviceSize allocation_size, 
        Memory_type_index memory_type_index, 
        std::optional<Allocation_callbacks> allocator
    ) noexcept;


    export void bind_memory(
        Device device,
        Buffer buffer,
        Device_memory memory,
        VkDeviceSize memory_offset
    ) noexcept;

    export void bind_memory(
        Device device,
        Image image,
        Device_memory memory,
        VkDeviceSize memory_offset
    ) noexcept;
}