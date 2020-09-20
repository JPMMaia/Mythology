export module maia.renderer.vulkan.device;

import <vulkan/vulkan.h>;

import <algorithm>;
import <cstdint>;
import <memory_resource>;
import <optional>;
import <span>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    export std::uint32_t get_physical_device_queue_family_count(VkPhysicalDevice physical_device) noexcept;
    export std::pmr::vector<VkQueueFamilyProperties> get_physical_device_queue_family_properties(VkPhysicalDevice physical_device, std::pmr::polymorphic_allocator<VkPhysicalDevice> const& allocator = {}) noexcept;

    export bool has_graphics_capabilities(VkQueueFamilyProperties const& queue_family_properties) noexcept;
    export bool has_compute_capabilities(VkQueueFamilyProperties const& queue_family_properties) noexcept;
    export bool has_transfer_capabilities(VkQueueFamilyProperties const& queue_family_properties) noexcept;


    export struct Queue_family_index
    {
        std::uint32_t value;
    };

    export bool operator==(Queue_family_index lhs, Queue_family_index rhs) noexcept;
    export bool operator!=(Queue_family_index lhs, Queue_family_index rhs) noexcept;

    
    export template <class Function>
    std::optional<Queue_family_index> find_queue_family_with_capabilities(
        std::span<VkQueueFamilyProperties const> const queue_family_properties,
        Function has_capabilities
    ) noexcept
    {
        auto const iterator = std::find_if(
            queue_family_properties.begin(), 
            queue_family_properties.end(),
            has_capabilities
        );

        if (iterator != queue_family_properties.end())
        {
            auto const index = std::distance(queue_family_properties.begin(), iterator);
            return {{static_cast<std::uint32_t>(index)}};
        }
        else 
        {
            return {};
        }
    }


    export struct VkDeviceQueueCreateInfo
    {
        VkDeviceQueueCreateInfo value;
    };

    export VkDeviceQueueCreateInfo create_device_queue_create_info(std::uint32_t queue_family_index, std::uint32_t queue_count, std::span<float const> queue_priorities) noexcept;


    export struct Device
    {
        VkDevice value = VK_NULL_HANDLE;
    };

    export Device create_device(VkPhysicalDevice physical_device, std::span<VkDeviceQueueCreateInfo const> queue_create_infos, std::span<char const* const> enabled_extensions) noexcept;
    export void destroy_device(Device device, VkAllocationCallbacks const* allocator = {}) noexcept;


    export struct Wait_device_idle_lock
    {
        Wait_device_idle_lock(Device const device) noexcept :
            device{device}
        {
        }
        Wait_device_idle_lock(Wait_device_idle_lock const&) noexcept = delete;
        Wait_device_idle_lock(Wait_device_idle_lock&&) noexcept = delete;
        ~Wait_device_idle_lock() noexcept
        {
            vkDeviceWaitIdle(device.value);
        }

        Wait_device_idle_lock& operator=(Wait_device_idle_lock const&) noexcept = delete;
        Wait_device_idle_lock& operator=(Wait_device_idle_lock&&) noexcept = delete;

        Device device;
    };
}