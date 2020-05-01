export module maia.renderer.vulkan.descriptor_pool;

import <vulkan/vulkan.h>;

namespace Maia::Renderer::Vulkan
{
    export VkDescriptorPool create_descriptor_pool(
        VkDevice device,
        VkDescriptorPoolCreateInfo const& create_info,
        VkAllocationCallbacks const* allocator = nullptr
    ) noexcept;

    export void destroy_descriptor_pool(
        VkDevice device,
        VkDescriptorPool const descriptor_pool,
        VkAllocationCallbacks const* allocator = nullptr
    ) noexcept;
}
