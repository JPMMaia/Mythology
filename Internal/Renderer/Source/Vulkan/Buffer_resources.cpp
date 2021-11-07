module;

#include <vulkan/vulkan.hpp>

#include <algorithm>
#include <cassert>
#include <memory_resource>
#include <ranges>
#include <span>
#include <vector>

module maia.renderer.vulkan.buffer_resources;

namespace Maia::Renderer::Vulkan
{
    Buffer_resources::Buffer_resources(
        vk::PhysicalDevice const physical_device,
        vk::Device const device,
        vk::PhysicalDeviceType const physical_device_type,
        vk::MemoryAllocateFlags const memory_allocate_flags,
        vk::DeviceSize const block_size,
        vk::BufferUsageFlags const usage,
        vk::SharingMode const sharing_mode,
        std::span<std::uint32_t const> const queue_family_indices,
        vk::AllocationCallbacks const* const allocation_callbacks,
        std::pmr::polymorphic_allocator<> const& allocator
    ) :
        m_physical_device{ physical_device },
        m_device{ device },
        m_physical_device_type{ physical_device_type },
        m_memory_allocate_flags{ memory_allocate_flags },
        m_usage{ usage },
        m_block_size{ block_size },
        m_sharing_mode{ sharing_mode },
        m_queue_family_indices{ queue_family_indices.begin(), queue_family_indices.end(), allocator },
        m_device_memory{ allocator },
        m_buffers{ allocator },
        m_memory_type_bits{ allocator },
        m_allocated_bytes{ allocator },
        m_allocation_callbacks{ allocation_callbacks }
    {
    }

    Buffer_resources::~Buffer_resources() noexcept
    {
        for (vk::DeviceMemory const memory : m_device_memory)
        {
            m_device.free(memory, m_allocation_callbacks);
        }

        for (vk::Buffer const buffer : m_buffers)
        {
            m_device.destroy(buffer, m_allocation_callbacks);
        }
    }

    namespace
    {
        std::optional<std::uint32_t> get_memory_type_index(
            vk::PhysicalDevice const physical_device,
            std::uint32_t bits,
            vk::MemoryPropertyFlags const property_flags
        ) noexcept
        {
            vk::PhysicalDeviceMemoryProperties const memory_properties = physical_device.getMemoryProperties();

            for (std::uint32_t memory_type_index = 0; memory_type_index < memory_properties.memoryTypeCount; ++memory_type_index)
            {
                if ((bits & 1) == 1)
                {
                    if ((memory_properties.memoryTypes[memory_type_index].propertyFlags & property_flags) == property_flags)
                    {
                        return memory_type_index;
                    }
                }

                bits >>= 1;
            }

            return std::nullopt;
        }

        struct Buffer_memory
        {
            vk::Buffer buffer = {};
            vk::DeviceMemory memory = {};
            std::uint32_t memory_type_bits = 0;
        };

        Buffer_memory create_buffer_and_memory(
            vk::PhysicalDevice const physical_device,
            vk::Device const device,
            vk::DeviceSize const block_size,
            vk::BufferUsageFlags const usage,
            vk::MemoryAllocateFlags const memory_allocate_flags,
            vk::MemoryPropertyFlags const required_memory_property_flags,
            vk::AllocationCallbacks const* const allocation_callbacks
        )
        {
            vk::BufferCreateInfo const buffer_create_info
            {
                .size = block_size,
                .usage = usage,
            };

            vk::Buffer const buffer = device.createBuffer(buffer_create_info, allocation_callbacks);

            vk::MemoryRequirements const memory_requirements = device.getBufferMemoryRequirements(buffer);

            vk::MemoryAllocateFlagsInfo const memory_allocate_flags_info
            {
                .flags = memory_allocate_flags,
            };

            std::optional<std::uint32_t> const memory_type_index = get_memory_type_index(
                physical_device,
                memory_requirements.memoryTypeBits,
                required_memory_property_flags
            );

            if (!memory_type_index)
            {
                device.destroy(buffer, allocation_callbacks);
                throw std::runtime_error{ "A memory type with the required properties was not found!" };
            }

            vk::MemoryAllocateInfo const memory_allocate_info
            {
                .pNext = &memory_allocate_flags_info,
                .allocationSize = memory_requirements.size,
                .memoryTypeIndex = *memory_type_index,
            };

            vk::DeviceMemory const memory = device.allocateMemory(memory_allocate_info, allocation_callbacks);

            vk::DeviceAddress const offset = 0;
            device.bindBufferMemory(buffer, memory, offset);

            return Buffer_memory
            {
                .buffer = buffer,
                .memory = memory,
                .memory_type_bits = memory_requirements.memoryTypeBits,
            };
        }
    }

    Buffer_view Buffer_resources::allocate_buffer(
        vk::DeviceSize const required_size,
        vk::MemoryPropertyFlags const required_memory_property_flags
    )
    {
        auto const has_free_space = [this, required_size](std::size_t const index) -> bool
        {
            vk::DeviceSize const allocated_bytes = m_allocated_bytes[index];

            return (allocated_bytes + required_size) <= m_block_size; // TODO align
        };

        auto const has_required_memory_properties = [this, required_memory_property_flags](std::size_t const index) -> bool
        {
            std::uint32_t const bits = m_memory_type_bits[index];

            std::optional<std::uint32_t> const memory_type_index = get_memory_type_index(m_physical_device, bits, required_memory_property_flags);

            return memory_type_index.has_value();
        };

        std::ranges::iota_view const indices_view{ std::size_t{0}, m_buffers.size() };

        auto const free_block_iterator = std::find_if(
            indices_view.begin(),
            indices_view.end(),
            [&](std::size_t const index) -> bool { return has_free_space(index) && has_required_memory_properties(index); }
        );

        if (free_block_iterator != indices_view.end())
        {
            auto const free_block_index = *free_block_iterator;

            vk::DeviceSize const offset = m_allocated_bytes[free_block_index]; // TODO align
            m_allocated_bytes[free_block_index] += required_size;

            Buffer_view const buffer_view
            {
                .memory = m_device_memory[free_block_index],
                .buffer = m_buffers[free_block_index],
                .offset = offset,
                .size = required_size,
            };

            return buffer_view;
        }
        else
        {
            Buffer_memory const buffer_memory = create_buffer_and_memory(
                m_physical_device,
                m_device,
                m_block_size,
                m_usage,
                m_memory_allocate_flags,
                required_memory_property_flags,
                m_allocation_callbacks
            );

            m_buffers.push_back(buffer_memory.buffer);
            m_device_memory.push_back(buffer_memory.memory);
            m_memory_type_bits.push_back(buffer_memory.memory_type_bits);
            m_allocated_bytes.push_back(required_size);

            Buffer_view const buffer_view
            {
                .memory = buffer_memory.memory,
                .buffer = buffer_memory.buffer,
                .offset = 0,
                .size = required_size,
            };

            return buffer_view;
        }
    }

    void Buffer_resources::clear() noexcept
    {
        std::fill(
            m_allocated_bytes.begin(),
            m_allocated_bytes.end(),
            vk::DeviceSize{ 0 }
        );
    }
}
