module maia.renderer.vulkan.instance;

import maia.renderer.vulkan.check;

import <vulkan/vulkan.h>;

import <cassert>;
import <ostream>;
import <memory_resource>;
import <optional>;
import <span>;
import <string_view>;

namespace Maia::Renderer::Vulkan
{
    std::ostream& operator<<(std::ostream& output_stream, VkLayerProperties const& layer_properties) noexcept
    {
        output_stream << "Layer name: " << layer_properties.layerName << '\n';
        output_stream << "Spec version: " << 
            VK_VERSION_MAJOR(layer_properties.specVersion) << '.' <<  
            VK_VERSION_MINOR(layer_properties.specVersion) << '.' << 
            VK_VERSION_PATCH(layer_properties.specVersion) << '\n';
        output_stream << "Implementation version: " << layer_properties.implementationVersion << '\n';
        output_stream << "Description: " << layer_properties.description << '\n';

        return output_stream;
    }

    std::pmr::vector<VkLayerProperties> enumerate_instance_layer_properties(std::pmr::polymorphic_allocator<VkLayerProperties> const& allocator) noexcept
    {
        uint32_t layer_property_count = 0;
        check_result(
            vkEnumerateInstanceLayerProperties(&layer_property_count, nullptr));

        std::pmr::vector<VkLayerProperties> layer_properties{layer_property_count, allocator};
        check_result(
            vkEnumerateInstanceLayerProperties(&layer_property_count, layer_properties.data()));

        return layer_properties;
    }

    API_version make_api_version(std::uint16_t const major, std::uint16_t const minor, std::uint16_t const patch) noexcept
    {
        return {static_cast<std::uint32_t>(VK_MAKE_VERSION(major, minor, patch))};
    }

    Instance create_instance(
        std::optional<Application_description> const application_description,
        std::optional<Engine_description> const engine_description,
        API_version const api_version,
        std::span<char const* const> const enabled_layers,
        std::span<char const* const> const enabled_extensions,
        void const* const next) noexcept
    {
        VkApplicationInfo const application_info
        {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = next,
            .pApplicationName = application_description ? application_description->name.data() : nullptr,
            .applicationVersion = application_description ? application_description->version : 0,
            .pEngineName = engine_description ? engine_description->name.data() : nullptr,
            .engineVersion = engine_description ? engine_description->version : 0,
            .apiVersion = api_version.value,
        };

        VkInstanceCreateInfo const info
        {
            .sType = VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .pApplicationInfo = &application_info,
            .enabledLayerCount = static_cast<std::uint32_t>(enabled_layers.size()),
            .ppEnabledLayerNames = enabled_layers.data(),
            .enabledExtensionCount = static_cast<std::uint32_t>(enabled_extensions.size()),
            .ppEnabledExtensionNames = enabled_extensions.data(),
        };

        VkInstance instance = {};
        check_result(
            vkCreateInstance(&info, nullptr, &instance));

        return { instance };
    }
}