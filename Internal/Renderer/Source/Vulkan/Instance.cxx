export module maia.renderer.vulkan.instance;


import <vulkan/vulkan.h>;

import <cstdint>;
import <iosfwd>;
import <memory_resource>;
import <optional>;
import <span>;
import <vector>;

namespace Maia::Renderer::Vulkan
{  
    export std::ostream& operator<<(std::ostream& output_stream, VkLayerProperties const& layer_properties) noexcept;

    export std::pmr::vector<VkLayerProperties> enumerate_instance_layer_properties(std::pmr::polymorphic_allocator<VkLayerProperties> const& allocator = {}) noexcept;

    export struct Application_description
    {
        char const* name;
        std::uint32_t version;
    };

    export struct Engine_description
    {
        char const* name;
        std::uint32_t version;
    };

    export struct API_version
    {
        std::uint32_t value;
    };

    export API_version make_api_version(std::uint16_t major, std::uint16_t minor, std::uint16_t patch) noexcept;

    export struct Instance
    {
        VkInstance value = VK_NULL_HANDLE;
    };

    export Instance create_instance(
        std::optional<Application_description> application_description,
        std::optional<Engine_description> engine_description,
        API_version api_version,
        std::span<char const* const> enabled_layers,
        std::span<char const* const> enabled_extensions,
        void const* next = nullptr) noexcept;

    export void destroy_instance(
        Instance instance,
        VkAllocationCallbacks const* allocator = {}
    ) noexcept;
}
