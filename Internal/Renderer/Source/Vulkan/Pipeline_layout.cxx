export module maia.renderer.vulkan.pipeline_layout;

import maia.renderer.vulkan.allocation_callbacks;
import maia.renderer.vulkan.device;

import <vulkan/vulkan.h>;

import <optional>;
import <span>;

namespace Maia::Renderer::Vulkan
{
    export VkPipelineLayout create_pipeline_layout(
        Device device,
        std::span<VkDescriptorSetLayout const> set_layouts,
        std::span<VkPushConstantRange const> push_constants,
        std::optional<Allocation_callbacks> vulkan_allocator = {}) noexcept;

    export void destroy_pipeline_layout(
        Device device,
        VkPipelineLayout pipeline_layout,
        std::optional<Allocation_callbacks> vulkan_allocator = {}
    ) noexcept;
}
