module maia.renderer.vulkan.physical_device;

import maia.renderer.vulkan.check;

import <vulkan/vulkan.h>;

import <memory_resource>;
import <optional>;
import <ostream>;
import <type_traits>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    std::ostream& operator<<(std::ostream& output_stream, VkPhysicalDeviceProperties const& physical_device_properties) noexcept
    {
        output_stream << "Device name: " << physical_device_properties.deviceName << '\n';
        output_stream << "Vulkan version: " << 
            VK_VERSION_MAJOR(physical_device_properties.apiVersion) << '.' <<  
            VK_VERSION_MINOR(physical_device_properties.apiVersion) << '.' << 
            VK_VERSION_PATCH(physical_device_properties.apiVersion) << '\n';
        output_stream << "Driver version: " << physical_device_properties.driverVersion << '\n';
        output_stream << "Vendor ID: " << physical_device_properties.vendorID << '\n';
        output_stream << "Device ID: " << physical_device_properties.deviceID << '\n';
        output_stream << "Device type: " << physical_device_properties.deviceType << '\n';

        return output_stream;
    }


    std::ostream& operator<<(std::ostream& output_stream, VkPhysicalDevice const physical_device) noexcept
    {
        VkPhysicalDeviceProperties properties = {};
        vkGetPhysicalDeviceProperties(physical_device, &properties);

        output_stream << properties;

        return output_stream;
    }

    std::pmr::vector<VkPhysicalDevice> enumerate_physical_devices(VkInstance const instance, std::pmr::polymorphic_allocator<VkPhysicalDevice> const& allocator) noexcept
    {
        std::uint32_t physical_device_count = 0;
        check_result(
            vkEnumeratePhysicalDevices(instance, &physical_device_count, NULL));

        std::pmr::vector<VkPhysicalDevice> physical_devices{physical_device_count, allocator};
        check_result(
            vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data()));

        return physical_devices;
    }


    Physical_device_features get_physical_device_properties(VkPhysicalDevice const physical_device) noexcept
    {
        VkPhysicalDeviceFeatures features = {};
        vkGetPhysicalDeviceFeatures(physical_device, &features);

        return { features };
    }

    std::pmr::vector<VkExtensionProperties> enumerate_physical_device_extension_properties(
        VkPhysicalDevice const physical_device,
        std::optional<char const*> const layer_name,
        std::pmr::polymorphic_allocator<VkPhysicalDevice> const& allocator
    ) noexcept
    {
        std::uint32_t property_count = 0;
        check_result(
            vkEnumerateDeviceExtensionProperties(
                physical_device,
                layer_name.has_value() ? *layer_name : nullptr,
                &property_count,
                nullptr
            )
        );

        std::pmr::vector<VkExtensionProperties> properties{property_count, allocator};
        check_result(
            vkEnumerateDeviceExtensionProperties(
                physical_device,
                layer_name.has_value() ? *layer_name : nullptr,
                &property_count,
                properties.data()
            )
        );

        return properties;
    }
}