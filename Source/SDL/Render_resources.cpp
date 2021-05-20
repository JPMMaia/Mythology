module;

#include <vulkan/vulkan.hpp>

#include <span>
#include <utility>

module mythology.sdl.render_resources;

import maia.renderer.vulkan;

namespace Mythology::Render
{
    using namespace Maia::Renderer::Vulkan;

    Instance_resources::Instance_resources(
        Maia::Renderer::Vulkan::API_version const api_version,
        std::span<char const* const> const required_instance_extensions
    ) :
        instance{create_instance(Application_description{"Mythology", 1}, Engine_description{"Mythology Engine", 1}, api_version, {}, required_instance_extensions, nullptr)}
    {
    }
    Instance_resources::~Instance_resources() noexcept
    {
        if (this->instance != VK_NULL_HANDLE)
        {
            Maia::Renderer::Vulkan::destroy_instance(this->instance);
        }
    }

    Instance_resources::Instance_resources(Instance_resources&& other) noexcept :
        instance{std::exchange(other.instance, VkInstance{VK_NULL_HANDLE})}
    {
    }

    Instance_resources& Instance_resources::operator=(Instance_resources&& other) noexcept
    {
        if (this->instance != VK_NULL_HANDLE)
        {
            Maia::Renderer::Vulkan::destroy_instance(this->instance);
        }

        this->instance = std::exchange(other.instance, VkInstance{VK_NULL_HANDLE});

        return *this;
    }


    /*Device_resources::Device_resources()
    {
        using namespace Maia::Renderer::Vulkan;

        this->instance = create_instance(
            Application_description{"Mythology", 1},
            Engine_description{"Mythology Engine", 1},
            api_version,
            required_instance_extensions
        );

        this->physical_device = select_physical_device(this->instance);
        
        this->graphics_queue_family_index = find_graphics_queue_family_index(this->physical_device);

        auto const is_extension_to_enable = [](VkExtensionProperties const& properties) -> bool
        {
            return std::strcmp(properties.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0;
        };

        if (this->graphics_queue_family_index != this->present_queue_family_index)
        {
            std::array<Queue_family_index, 2> const queue_family_indices{this->graphics_queue_family_index, this->present_queue_family_index};
            this->device = create_device(this->physical_device, queue_family_indices, is_extension_to_enable);
        }
        else
        {
            Queue_family_index const queue_family_index = this->present_queue_family_index;
            this->device = create_device(this->physical_device, {&queue_family_index, 1}, is_extension_to_enable);
        }
    }

    Device_resources::~Device_resources() noexcept
    {
        if (this->device != VK_NULL_HANDLE)
        {
            destroy_device(this->device);
        }

        if (this->surface != VK_NULL_HANDLE)
        {
            destroy_surface(this->instance, this->surface);
        }

        if (this->instance != VK_NULL_HANDLE)
        {
            destroy_instance(this->instance);
        }
    }*/
}
