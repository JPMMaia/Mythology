module;

#include <vulkan/vulkan.hpp>

#include <cstdint>
#include <memory_resource>
#include <span>
#include <vector>

export module maia.renderer.vulkan.buffer_resources;

namespace Maia::Renderer::Vulkan
{
    export struct Buffer_view
    {
        vk::DeviceMemory memory = {};
        vk::Buffer buffer = {};
        vk::DeviceSize offset = 0;
        vk::DeviceSize size = 0;
    };

    export class Buffer_resources
    {
    public:

        Buffer_resources(
            vk::PhysicalDevice const physical_device,
            vk::Device device,
            vk::PhysicalDeviceType physical_device_type,
            vk::MemoryAllocateFlags const memory_allocate_flags = {},
            vk::DeviceSize block_size = 64 * 1024 * 1024,
            vk::BufferUsageFlags usage = {},
            vk::SharingMode sharing_mode = {},
            std::span<std::uint32_t const> queue_family_indices = {},
            vk::AllocationCallbacks const* allocation_callbacks = nullptr,
            std::pmr::polymorphic_allocator<> const& allocator = {}
        );
        Buffer_resources(Buffer_resources const&) = delete;
        Buffer_resources(Buffer_resources&&) noexcept = default;
        ~Buffer_resources() noexcept;

        Buffer_resources& operator=(Buffer_resources const&) = delete;
        Buffer_resources& operator=(Buffer_resources&&) noexcept = default;

        Buffer_view allocate_buffer(
            vk::DeviceSize const required_size,
            std::uint32_t const required_alignment,
            vk::MemoryPropertyFlags required_memory_property_flags = vk::MemoryPropertyFlagBits::eDeviceLocal
        );

        void clear() noexcept;

        vk::BufferUsageFlags usage() noexcept;

    private:

        vk::PhysicalDevice m_physical_device = {};
        vk::Device m_device = {};
        vk::PhysicalDeviceType m_physical_device_type = {};
        vk::MemoryAllocateFlags m_memory_allocate_flags = {};
        vk::BufferUsageFlags m_usage = {};
        vk::DeviceSize m_block_size = 0;
        vk::SharingMode m_sharing_mode = {};
        std::pmr::vector<std::uint32_t> m_queue_family_indices = {};
        std::pmr::vector<vk::DeviceMemory> m_device_memory;
        std::pmr::vector<vk::Buffer> m_buffers;
        std::pmr::vector<std::uint32_t> m_memory_type_bits;
        std::pmr::vector<vk::DeviceSize> m_allocated_bytes;
        vk::AllocationCallbacks const* m_allocation_callbacks = nullptr;

    };
}
