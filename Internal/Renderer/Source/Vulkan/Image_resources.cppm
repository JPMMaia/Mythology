module;

#include <vulkan/vulkan.hpp>

#include <cstdint>
#include <memory_resource>
#include <span>
#include <vector>

export module maia.renderer.vulkan.image_resources;

namespace Maia::Renderer::Vulkan
{
    export struct Image_memory_view
    {
        vk::DeviceMemory memory = {};
        vk::Image image = {};
        vk::DeviceSize offset = 0;
        vk::DeviceSize size = 0;
    };

    struct Image_memory_properties
    {
        std::uint32_t memory_type_index = {};
        vk::MemoryPropertyFlags memory_properties = {};
    };

    export class Image_resources
    {
    public:

        Image_resources(
            vk::PhysicalDevice const physical_device,
            vk::Device device,
            vk::PhysicalDeviceType physical_device_type,
            vk::MemoryAllocateFlags const memory_allocate_flags = {},
            vk::DeviceSize block_size = 64 * 1024 * 1024,
            vk::SharingMode sharing_mode = {},
            std::span<std::uint32_t const> queue_family_indices = {},
            vk::AllocationCallbacks const* allocation_callbacks = nullptr,
            std::pmr::polymorphic_allocator<> const& allocator = {}
        );
        Image_resources(Image_resources const&) = delete;
        Image_resources(Image_resources&&) noexcept = default;
        ~Image_resources() noexcept;

        Image_resources& operator=(Image_resources const&) = delete;
        Image_resources& operator=(Image_resources&&) noexcept = default;

        Image_memory_view allocate_image(
            vk::ImageCreateInfo const& create_info,
            vk::MemoryPropertyFlags required_memory_property_flags = vk::MemoryPropertyFlagBits::eDeviceLocal
        );

    private:

        vk::PhysicalDevice m_physical_device = {};
        vk::Device m_device = {};
        vk::PhysicalDeviceType m_physical_device_type = {};
        vk::MemoryAllocateFlags m_memory_allocate_flags = {};
        vk::DeviceSize m_block_size = 0;
        vk::SharingMode m_sharing_mode = {};
        std::pmr::vector<std::uint32_t> m_queue_family_indices = {};
        std::pmr::vector<vk::DeviceMemory> m_device_memory;
        std::pmr::vector<vk::Image> m_images;
        std::pmr::vector<Image_memory_properties> m_memory_properties;
        std::pmr::vector<vk::DeviceSize> m_allocated_bytes;
        vk::AllocationCallbacks const* m_allocation_callbacks = nullptr;

    };
}
