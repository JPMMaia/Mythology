module;

#include <vulkan/vulkan.hpp>

#include <algorithm>
#include <cassert>
#include <memory_resource>
#include <ranges>
#include <span>
#include <vector>

module maia.renderer.vulkan.image_resources;

namespace Maia::Renderer::Vulkan
{
    Image_resources::Image_resources(
        vk::PhysicalDevice const physical_device,
        vk::Device const device,
        vk::PhysicalDeviceType const physical_device_type,
        vk::MemoryAllocateFlags const memory_allocate_flags,
        vk::DeviceSize const block_size,
        vk::SharingMode const sharing_mode,
        std::span<std::uint32_t const> const queue_family_indices,
        vk::AllocationCallbacks const* const allocation_callbacks,
        std::pmr::polymorphic_allocator<> const& allocator
    ) :
        m_physical_device{ physical_device },
        m_device{ device },
        m_physical_device_type{ physical_device_type },
        m_memory_allocate_flags{ memory_allocate_flags },
        m_block_size{ block_size },
        m_sharing_mode{ sharing_mode },
        m_queue_family_indices{ queue_family_indices.begin(), queue_family_indices.end(), allocator },
        m_device_memory{ allocator },
        m_images{ allocator },
        m_memory_properties{ allocator },
        m_allocated_bytes{ allocator },
        m_allocation_callbacks{ allocation_callbacks }
    {
    }

    Image_resources::~Image_resources() noexcept
    {
        for (vk::DeviceMemory const memory : m_device_memory)
        {
            m_device.free(memory, m_allocation_callbacks);
        }

        for (vk::Image const image : m_images)
        {
            m_device.destroy(image, m_allocation_callbacks);
        }
    }

    namespace
    {
        std::optional<Image_memory_properties> find_memory_type(
            vk::PhysicalDevice const physical_device,
            std::uint32_t const required_memory_type_bits,
            vk::MemoryPropertyFlags const required_properties
        ) noexcept
        {
            vk::PhysicalDeviceMemoryProperties const memory_properties = physical_device.getMemoryProperties();

            for (std::uint32_t memory_index = 0; memory_index < memory_properties.memoryTypeCount; ++memory_index)
            {
                std::uint32_t const memory_type_bits = (1 << memory_index);
                bool const is_required_memory_type = memory_type_bits & required_memory_type_bits;

                vk::MemoryPropertyFlags const properties = memory_properties.memoryTypes[memory_index].propertyFlags;
                bool const has_required_properties = (properties & required_properties) == required_properties;

                if (is_required_memory_type && has_required_properties)
                {
                    return Image_memory_properties
                    {
                        .memory_type_index = memory_index,
                        .memory_properties = properties,
                    };
                }
            }

            return std::nullopt;
        }

        struct Device_memory_and_properties
        {
            vk::DeviceMemory memory;
            Image_memory_properties properties;
        };

        std::optional<Device_memory_and_properties> allocate_device_memory(
            vk::PhysicalDevice const physical_device,
            vk::Device const device,
            vk::DeviceSize const block_size,
            std::uint32_t const required_memory_type_bits,
            vk::MemoryAllocateFlags const memory_allocate_flags,
            vk::MemoryPropertyFlags const required_memory_property_flags,
            vk::AllocationCallbacks const* const allocation_callbacks
        )
        {
            std::optional<Image_memory_properties> const memory_properties = find_memory_type(
                physical_device,
                required_memory_type_bits,
                required_memory_property_flags
            );

            if (!memory_properties)
            {
                return std::nullopt;
            }

            vk::MemoryAllocateFlagsInfo const memory_allocate_flags_info
            {
                .flags = memory_allocate_flags,
            };

            vk::MemoryAllocateInfo const memory_allocate_info
            {
                .pNext = &memory_allocate_flags_info,
                .allocationSize = block_size,
                .memoryTypeIndex = memory_properties->memory_type_index,
            };

            vk::DeviceMemory const memory = device.allocateMemory(memory_allocate_info, allocation_callbacks);

            return Device_memory_and_properties
            {
                .memory = memory,
                .properties = *memory_properties,
            };
        }

        template <typename T, typename S>
        T align(T const value, S const alignment) noexcept
        {
            T const remainder = (value % alignment);
            T const aligned_value = (remainder == 0) ? value : (value + (alignment - remainder));
            assert((aligned_value % alignment) == 0);
            return aligned_value;
        }
    }

    Image_memory_view Image_resources::allocate_image(
        vk::ImageCreateInfo const& create_info,
        vk::MemoryPropertyFlags const required_memory_property_flags
    )
    {
        vk::Image const image = m_device.createImage(create_info, m_allocation_callbacks);

        vk::MemoryRequirements2 const memory_requirements = m_device.getImageMemoryRequirements2(
            vk::ImageMemoryRequirementsInfo2{ .image = image }
        );

        vk::DeviceSize const required_size = memory_requirements.memoryRequirements.size;
        vk::DeviceSize const required_alignment = memory_requirements.memoryRequirements.alignment;
        std::uint32_t const required_memory_type_bits = memory_requirements.memoryRequirements.memoryTypeBits;

        auto const has_free_space = [this, required_size, required_alignment](std::size_t const index) -> bool
        {
            vk::DeviceSize const allocated_bytes = m_allocated_bytes[index];

            return (align(allocated_bytes, required_alignment) + required_size) <= m_block_size;
        };

        auto const has_required_memory_properties = [this, required_memory_type_bits, required_memory_property_flags](std::size_t const index) -> bool
        {
            Image_memory_properties const actual_memory_properties = m_memory_properties[index];
            std::uint32_t const actual_memory_type_index = (1 << actual_memory_properties.memory_type_index);

            return
                ((required_memory_type_bits & actual_memory_type_index) != 0) &&
                ((actual_memory_properties.memory_properties & required_memory_property_flags) == required_memory_property_flags);
        };

        std::ranges::iota_view const indices_view{ std::size_t{0}, m_device_memory.size() };

        auto const free_block_iterator = std::find_if(
            indices_view.begin(),
            indices_view.end(),
            [&](std::size_t const index) -> bool { return has_free_space(index) && has_required_memory_properties(index); }
        );

        if (free_block_iterator != indices_view.end())
        {
            auto const free_block_index = *free_block_iterator;

            vk::DeviceMemory const memory = m_device_memory[free_block_index];
            vk::DeviceSize const memory_offset = align(m_allocated_bytes[free_block_index], required_alignment);

            m_device.bindImageMemory(image, memory, memory_offset);

            m_images.push_back(image);
            m_allocated_bytes[free_block_index] += memory_offset + required_size;

            Image_memory_view const image_memory_view
            {
                .memory = memory,
                .image = image,
                .offset = memory_offset,
                .size = required_size,
            };

            return image_memory_view;
        }
        else
        {
            std::optional<Device_memory_and_properties> const memory_and_properties = allocate_device_memory(
                m_physical_device,
                m_device,
                m_block_size,
                required_memory_type_bits,
                m_memory_allocate_flags,
                required_memory_property_flags,
                m_allocation_callbacks
            );

            if (!memory_and_properties)
            {
                m_device.destroy(image, m_allocation_callbacks);
                throw std::runtime_error{ "A memory type with the required properties was not found!" };
            }

            vk::DeviceMemory const memory = memory_and_properties->memory;
            vk::DeviceSize const memory_offset = 0;
            m_device.bindImageMemory(image, memory, memory_offset);

            m_images.push_back(image);
            m_device_memory.push_back(memory);
            m_memory_properties.push_back(memory_and_properties->properties);
            m_allocated_bytes.push_back(required_size);

            Image_memory_view const image_memory_view
            {
                .memory = memory,
                .image = image,
                .offset = memory_offset,
                .size = required_size,
            };

            return image_memory_view;
        }
    }
}
