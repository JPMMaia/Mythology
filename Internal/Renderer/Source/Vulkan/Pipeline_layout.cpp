module maia.renderer.vulkan.pipeline_layout;

import maia.renderer.vulkan.allocation_callbacks;
import maia.renderer.vulkan.check;
import maia.renderer.vulkan.device;

import <vulkan/vulkan.h>;

import <optional>;
import <span>;

namespace Maia::Renderer::Vulkan
{
    VkPipelineLayout create_pipeline_layout(
        Device const device,
        std::span<VkDescriptorSetLayout const> const set_layouts,
        std::span<VkPushConstantRange const> const push_constants,
        std::optional<Allocation_callbacks> const vulkan_allocator) noexcept
    {
        VkPipelineLayoutCreateInfo const create_info
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .setLayoutCount = static_cast<std::uint32_t>(set_layouts.size()),
            .pSetLayouts = set_layouts.data(),
            .pushConstantRangeCount = static_cast<std::uint32_t>(push_constants.size()),
            .pPushConstantRanges = push_constants.data(),
        };

        VkPipelineLayout pipeline_layout = {};
        check_result(
            vkCreatePipelineLayout(
                device.value,
                &create_info,
                vulkan_allocator.has_value() ? &vulkan_allocator->value : nullptr,
                &pipeline_layout
            )
        );

        return pipeline_layout;
    }

    void destroy_pipeline_layout(
        Device const device,
        VkPipelineLayout const pipeline_layout,
        std::optional<Allocation_callbacks> const vulkan_allocator
    ) noexcept
    {
        vkDestroyPipelineLayout(
            device.value,
            pipeline_layout,
            vulkan_allocator.has_value() ? &vulkan_allocator->value : nullptr
        );
    }
}
