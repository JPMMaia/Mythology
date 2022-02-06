module;

#include <vulkan/vulkan.hpp>

export module maia.renderer.vulkan.command_buffer;

namespace Maia::Renderer::Vulkan
{
    export vk::CommandBuffer create_one_time_submit_command_buffer(
        vk::Device device,
        vk::CommandPool command_pool
    ) noexcept;

    export void submit(
        vk::Device device,
        vk::Queue queue,
        vk::CommandPool command_pool,
        vk::CommandBuffer command_buffer,
        vk::AllocationCallbacks const* allocation_callbacks
    );
}
