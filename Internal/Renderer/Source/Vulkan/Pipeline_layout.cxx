module;

#include <vulkan/vulkan.h>

export module maia.renderer.vulkan.pipeline_layout;
namespace Maia::Renderer::Vulkan
{
    export VkPipelineLayoutCreateInfo constexpr empty_pipeline_layout_create_info() noexcept
    {
        return 
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .setLayoutCount = 0,
            .pSetLayouts = nullptr,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr,
        };
    }

    export VkPipelineLayout create_pipeline_layout(
        VkDevice device,
        VkPipelineLayoutCreateInfo const& create_info,
        VkAllocationCallbacks const* allocator = nullptr
    ) noexcept;

    export void destroy_pipeline_layout(
        VkDevice device,
        VkPipelineLayout pipeline_layout,
        VkAllocationCallbacks const* allocator = nullptr
    ) noexcept;
}
