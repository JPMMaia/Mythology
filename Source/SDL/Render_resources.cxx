module;

#include <vulkan/vulkan.hpp>

#include <memory_resource>
#include <optional>
#include <span>
#include <vector>

export module mythology.sdl.render_resources;

import maia.renderer.vulkan;

namespace Mythology::Render
{
    export struct Instance_resources
    {
        Instance_resources(
            Maia::Renderer::Vulkan::API_version api_version,
            std::span<char const* const> required_instance_extensions
        );
        Instance_resources(Instance_resources const&) = delete;
        Instance_resources(Instance_resources&& other) noexcept;
        ~Instance_resources() noexcept;

        Instance_resources& operator=(Instance_resources const&) = delete;
        Instance_resources& operator=(Instance_resources&& other) noexcept;

        VkInstance instance = VK_NULL_HANDLE;
    };

    export struct Queue_familiy_indices
    {
        std::optional<Maia::Renderer::Vulkan::Queue_family_index> compute;
        std::optional<Maia::Renderer::Vulkan::Queue_family_index> graphics;
        std::optional<Maia::Renderer::Vulkan::Queue_family_index> present;
        std::optional<Maia::Renderer::Vulkan::Queue_family_index> transfer;
    };

    export struct Physical_device_resources
    {
        VkPhysicalDevice phyisical_device;
        Queue_familiy_indices queue_family_indices;
    };

    export struct Device_configuration
    {

    };

    export struct Device_resources
    {
        Device_resources(Device_configuration const& configuration);
        Device_resources(Device_resources const&) = delete;
        Device_resources(Device_resources&& other) noexcept;
        ~Device_resources() noexcept;

        Device_resources& operator=(Device_resources const&) = delete;
        Device_resources& operator=(Device_resources&& other) noexcept;

        VkDevice device = VK_NULL_HANDLE;
    };
}
