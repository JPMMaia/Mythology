module;

#include <vulkan/vulkan.h>

#include <optional>
#include <span>

module maia.renderer.vulkan.pipeline_layout;

import maia.renderer.vulkan.check;
namespace Maia::Renderer::Vulkan
{
    VkPipelineLayout create_pipeline_layout(
        VkDevice const device,
        VkPipelineLayoutCreateInfo const& create_info,
        VkAllocationCallbacks const* const allocator
    ) noexcept
    {
        VkPipelineLayout pipeline_layout{};

        check_result(
            vkCreatePipelineLayout(
                device,
                &create_info,
                allocator,
                &pipeline_layout
            )
        );

        return pipeline_layout;
    }

    void destroy_pipeline_layout(
        VkDevice const device,
        VkPipelineLayout const pipeline_layout,
        VkAllocationCallbacks const* const allocator
    ) noexcept
    {
        vkDestroyPipelineLayout(
            device,
            pipeline_layout,
            allocator
        );
    }
}
