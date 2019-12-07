export module maia.renderer.vulkan.device_memory;

import maia.renderer.vulkan.physical_device;

import <vulkan/vulkan.h>;

import <cstdint>;
import <optional>;
import <ostream>;

namespace Maia::Renderer::Vulkan
{
    export struct Physical_device_memory_properties
    {
        VkPhysicalDeviceMemoryProperties value;
    };

    export std::ostream& operator<<(std::ostream& output_stream, Physical_device_memory_properties const& physical_device_memory_properties) noexcept;


    export struct Memory_type_index
    {
        std::uint32_t value;
    };

    export std::optional<Memory_type_index> find_memory_type(
        Physical_device_memory_properties const& memory_properties,
        std::uint32_t memory_type_bits_requirement,
        VkMemoryPropertyFlags required_properties
    ) noexcept;

    export std::optional<Memory_type_index> find_memory_type(
        Physical_device_memory_properties const& memory_properties,
        std::uint32_t memory_type_bits_requirement,
        VkMemoryPropertyFlags required_properties,
        VkMemoryPropertyFlags optimal_properties
    ) noexcept;


    export struct Device_memory
    {
        VkDeviceMemory value;
    };

    export Physical_device_memory_properties get_phisical_device_memory_properties(Physical_device const physical_device) noexcept;

}