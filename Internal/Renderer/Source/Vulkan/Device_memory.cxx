module;

#include <vulkan/vulkan.h>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <functional>
#include <ostream>
#include <span>

export module maia.renderer.vulkan.device_memory;
namespace Maia::Renderer::Vulkan
{
    export VkMemoryRequirements get_memory_requirements(
        VkDevice device,
        VkBuffer buffer
    ) noexcept;

    export VkMemoryRequirements get_memory_requirements(
        VkDevice device,
        VkImage image
    ) noexcept;


    export struct Memory_type_bits
    {
        std::uint32_t value;
    };

    export Memory_type_bits get_memory_type_bits(
        VkMemoryRequirements memory_requirements
    ) noexcept;


    export std::ostream& operator<<(
        std::ostream& output_stream, 
        VkPhysicalDeviceMemoryProperties const& physical_device_memory_properties
    ) noexcept;

    export VkPhysicalDeviceMemoryProperties get_phisical_device_memory_properties(
        VkPhysicalDevice const physical_device
    ) noexcept;


    export struct Memory_type_index
    {
        std::uint32_t value;
    };

    export bool operator==(Memory_type_index lhs, Memory_type_index rhs) noexcept
    {
        return lhs.value == rhs.value;
    }
    
    export bool operator!=(Memory_type_index lhs, Memory_type_index rhs) noexcept
    {
        return !(lhs == rhs);
    }
    

    export struct Memory_type_index_and_properties
    {
        Memory_type_index type_index;
        VkMemoryPropertyFlags properties;
    };

    export std::optional<Memory_type_index_and_properties> find_memory_type(
        VkPhysicalDeviceMemoryProperties const& memory_properties,
        Memory_type_bits memory_type_bits_requirement,
        VkMemoryPropertyFlags required_properties
    ) noexcept;

    export std::optional<Memory_type_index_and_properties> find_memory_type(
        VkPhysicalDeviceMemoryProperties const& memory_properties,
        Memory_type_bits memory_type_bits_requirement,
        VkMemoryPropertyFlags required_properties,
        VkMemoryPropertyFlags optimal_properties
    ) noexcept;


    export VkDeviceMemory allocate_memory(
        VkDevice device, 
        VkDeviceSize allocation_size, 
        Memory_type_index memory_type_index, 
        VkAllocationCallbacks const* allocator = nullptr
    ) noexcept;

    export void free_memory(
        VkDevice device,
        VkDeviceMemory device_memory,
        VkAllocationCallbacks const* allocator = nullptr
    ) noexcept;


    export void bind_memory(
        VkDevice device,
        VkBuffer buffer,
        VkDeviceMemory memory,
        VkDeviceSize memory_offset
    ) noexcept;

    export void bind_memory(
        VkDevice device,
        VkImage image,
        VkDeviceMemory memory,
        VkDeviceSize memory_offset
    ) noexcept;

    export void flush_mapped_memory_ranges(
        VkDevice device,
        std::span<VkMappedMemoryRange const> memory_ranges
    ) noexcept;


    export void* map_memory(
        VkDevice device,
        VkDeviceMemory device_memory,
        VkDeviceSize offset,
        VkDeviceSize size,
        VkMemoryMapFlags flags = {}
    ) noexcept;

    export void unmap_memory(
        VkDevice device,
        VkDeviceMemory device_memory
    ) noexcept;

    export class Mapped_memory
    {
    public:

        Mapped_memory(
            VkDevice device,
            VkDeviceMemory device_memory,
            VkDeviceSize offset,
            VkDeviceSize size,
            VkMemoryMapFlags flags = {}
        ) noexcept;
        Mapped_memory(Mapped_memory const&) noexcept = delete;
        Mapped_memory(Mapped_memory&& other) noexcept;
        ~Mapped_memory() noexcept;

        Mapped_memory& operator=(Mapped_memory const&) noexcept = delete;
        Mapped_memory& operator=(Mapped_memory&& other) noexcept;

        void* data() const noexcept;

    private:

        VkDevice m_device = {};
        VkDeviceMemory m_device_memory = {};
        void* m_mapped_memory = nullptr;
    };

    export struct Memory_type_index_hash
    {
        using value_type = Maia::Renderer::Vulkan::Memory_type_index;

        std::size_t operator()(value_type const value) const noexcept
        {
            return std::hash<std::uint32_t>{}(value.value);
        }
    };
}
