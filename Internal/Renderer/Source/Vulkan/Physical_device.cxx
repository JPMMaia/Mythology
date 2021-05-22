module;

#include <vulkan/vulkan.h>

#include <iosfwd>
#include <memory_resource>
#include <optional>
#include <vector>

export module maia.renderer.vulkan.physical_device;
namespace Maia::Renderer::Vulkan
{
    export std::ostream& operator<<(std::ostream& output_stream, VkPhysicalDeviceProperties const& physical_device_properties) noexcept;

    export VkPhysicalDeviceProperties get_physical_device_properties(VkPhysicalDevice physical_device) noexcept;


    export std::ostream& operator<<(std::ostream& output_stream, VkPhysicalDevice physical_device) noexcept;

    export std::pmr::vector<VkPhysicalDevice> enumerate_physical_devices(VkInstance instance, std::pmr::polymorphic_allocator<VkPhysicalDevice> const& allocator = {}) noexcept;


    export VkPhysicalDeviceFeatures get_physical_device_features(VkPhysicalDevice physical_device) noexcept;

    export std::pmr::vector<VkExtensionProperties> enumerate_physical_device_extension_properties(
        VkPhysicalDevice physical_device,
        std::optional<char const*> layer_name,
        std::pmr::polymorphic_allocator<VkPhysicalDevice> const& allocator = {}
    ) noexcept;
}