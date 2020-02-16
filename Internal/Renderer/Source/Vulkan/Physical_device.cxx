export module maia.renderer.vulkan.physical_device;

import maia.renderer.vulkan.instance;

import <vulkan/vulkan.h>;

import <iosfwd>;
import <memory_resource>;
import <optional>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    export std::ostream& operator<<(std::ostream& output_stream, VkPhysicalDeviceProperties const& physical_device_properties) noexcept;


    export struct Physical_device
    {
        VkPhysicalDevice value;
    };

    export std::ostream& operator<<(std::ostream& output_stream, Physical_device physical_device) noexcept;

    export std::pmr::vector<Physical_device> enumerate_physical_devices(Instance instance, std::pmr::polymorphic_allocator<Physical_device> const& allocator = {}) noexcept;


    export struct Physical_device_features
    {
        VkPhysicalDeviceFeatures value;
    };

    export Physical_device_features get_physical_device_properties(Physical_device physical_device) noexcept;

    export std::pmr::vector<VkExtensionProperties> enumerate_physical_device_extension_properties(
        Physical_device physical_device,
        std::optional<char const*> layer_name,
        std::pmr::polymorphic_allocator<Physical_device> const& allocator = {}
    ) noexcept;
}