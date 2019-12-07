module maia.renderer.vulkan.instance;

import maia.renderer.vulkan.check;

import <vulkan/vulkan.h>;

import <cassert>;
import <ostream>;
import <memory_resource>;
import <span>;

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

    Instance create_instance(std::span<char const* const> const enabled_layers, std::span<char const* const> const enabled_extensions) noexcept
    {
        VkInstanceCreateInfo info = {};
        info.sType = VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = {};
        info.pApplicationInfo = nullptr;
        info.enabledLayerCount = enabled_layers.size();
        info.ppEnabledLayerNames = enabled_layers.data();
        info.enabledExtensionCount = enabled_extensions.size();
        info.ppEnabledExtensionNames = enabled_extensions.data();

        VkInstance instance = {};
        check_result(
            vkCreateInstance(&info, nullptr, &instance));

        return { instance };
    }
}