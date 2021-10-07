module;

#include <vulkan/vulkan.hpp>

#include <optional>
#include <span>

export module maia.renderer.vulkan.upload;

import maia.renderer.vulkan.buffer_resources;

namespace Maia::Renderer::Vulkan
{
    export class Upload_buffer
    {
    public:

        Upload_buffer(
            vk::Device device,
            Buffer_resources& buffer_resources,
            vk::DeviceSize size
        );
        Upload_buffer(Upload_buffer const&) = delete;
        Upload_buffer(Upload_buffer&& other) noexcept;
        ~Upload_buffer();

        Upload_buffer& operator=(Upload_buffer const& other) = delete;
        Upload_buffer& operator=(Upload_buffer&& other) noexcept;

        Buffer_view buffer_view() const noexcept;
        void* mapped_data() const noexcept;

    private:

        vk::Device m_device = {};
        Buffer_view m_buffer_view = {};
        void* m_mapped_data = nullptr;

    };

    export void upload_data(
        vk::PhysicalDeviceType physical_device_type,
        vk::Device device,
        vk::Queue queue,
        vk::CommandPool command_pool,
        Buffer_view const& buffer_view,
        std::span<std::byte const> data,
        std::optional<Upload_buffer const*> upload_buffer,
        vk::AllocationCallbacks const* allocation_callbacks
    );
}
