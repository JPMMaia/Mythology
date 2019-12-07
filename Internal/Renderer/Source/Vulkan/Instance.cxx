export module maia.renderer.vulkan.instance;

import <vulkan/vulkan.h>;

import <memory_resource>;
import <iosfwd>;
import <span>;
import <vector>;

namespace Maia::Renderer::Vulkan
{  
    export std::ostream& operator<<(std::ostream& output_stream, VkLayerProperties const& layer_properties) noexcept;

    export std::pmr::vector<VkLayerProperties> enumerate_instance_layer_properties(std::pmr::polymorphic_allocator<VkLayerProperties> const& allocator = {}) noexcept;

    export struct Instance
    {
        VkInstance value;
    };

    export Instance create_instance(std::span<char const* const> const enabled_layers, std::span<char const* const> enabled_extensions) noexcept;
}
