export module maia.renderer.vulkan.device;

import maia.renderer.vulkan.physical_device;

import <vulkan/vulkan.h>;

import <cstdint>;
import <memory_resource>;
import <span>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    export struct Queue_family_properties
    {
        VkQueueFamilyProperties value;
    };

    export std::pmr::vector<Queue_family_properties> get_physical_device_queue_family_properties(Physical_device physical_device, std::pmr::polymorphic_allocator<Physical_device> const& allocator = {}) noexcept;

    export bool has_graphics_capabilities(Queue_family_properties const& queue_family_properties) noexcept;
    export bool has_compute_capabilities(Queue_family_properties const& queue_family_properties) noexcept;
    export bool has_transfer_capabilities(Queue_family_properties const& queue_family_properties) noexcept;


    export struct Device_queue_create_info
    {
        VkDeviceQueueCreateInfo value;
    };

    export Device_queue_create_info create_device_queue_create_info(std::uint32_t queue_family_index, std::uint32_t queue_count, std::span<float const> queue_priorities) noexcept;


    export struct Device
    {
        VkDevice value;
    };

    export Device create_device(Physical_device physical_device, std::span<Device_queue_create_info const> queue_create_infos, std::span<char const* const> enabled_extensions) noexcept;
}