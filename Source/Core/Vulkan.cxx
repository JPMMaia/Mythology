export module mythology.core.vulkan;

import maia.renderer.vulkan;

import <vulkan/vulkan.h>;

import <cstddef>;
import <fstream>;
import <functional>;
import <memory_resource>;
import <optional>;
import <span>;
import <vector>;

using namespace Maia::Renderer::Vulkan;

namespace Mythology::Core::Vulkan
{
    export Instance create_instance(
        std::optional<Application_description> application_description,
        std::optional<Engine_description> engine_description,
        API_version api_version,
        std::span<char const* const> required_extensions = {}) noexcept;
    
    export Physical_device select_physical_device(Instance instance) noexcept;

    export Queue_family_index find_graphics_queue_family_index(
        Physical_device physical_device
    ) noexcept;

    export Queue_family_index find_present_queue_family_index(
        Physical_device physical_device,
        Surface surface,
        std::optional<Queue_family_index> preference = {}
    ) noexcept;
    
    export Device create_device(
        Physical_device physical_device,
        std::span<Queue_family_index const> queue_family_indices,
        std::function<bool(VkExtensionProperties)> const& is_extension_to_enable) noexcept;

    export struct Device_memory_and_color_image
    {
        Device_memory device_memory;
        Image color_image;
    };

    export Device_memory_and_color_image create_device_memory_and_color_image(
        Physical_device physical_device,
        Device device,
        VkFormat format,
        VkExtent3D extent
    ) noexcept;

    export void render(
        Command_buffer command_buffer,
        Image output_image,
        VkClearColorValue clear_color,
        bool const switch_to_present_layout = false
    ) noexcept;

    export std::pmr::vector<std::byte> read_memory(
        Device device,
        Device_memory device_memory,
        VkSubresourceLayout subresource_layout,
        std::pmr::polymorphic_allocator<std::byte> const& allocator = {}
    ) noexcept;

    export void write_p3(
        std::ostream& output_stream,
        std::span<std::byte const> data_to_write,
        VkSubresourceLayout subresource_layout,
        VkExtent3D subresource_extent
    ) noexcept;
}