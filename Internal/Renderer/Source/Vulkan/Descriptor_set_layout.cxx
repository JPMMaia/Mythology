module;

#include <vulkan/vulkan.h>

export module maia.renderer.vulkan.descriptor_set_layout;
namespace Maia::Renderer::Vulkan
{
    export VkDescriptorSetLayout create_descriptor_set_layout(
        VkDevice device,
        VkDescriptorSetLayoutCreateInfo const& create_info,
        VkAllocationCallbacks const* allocator = nullptr
    ) noexcept;

    export void destroy_descriptor_set_layout(
        VkDevice device,
        VkDescriptorSetLayout descriptor_set_layout,
        VkAllocationCallbacks const* allocator = nullptr
    ) noexcept;
}
