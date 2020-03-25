export module maia.renderer.vulkan.pipeline;

import maia.renderer.vulkan.allocation_callbacks;
import maia.renderer.vulkan.device;

import <vulkan/vulkan.h>;

import <memory_resource>;
import <optional>;
import <span>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    export std::pmr::vector<VkPipeline> create_compute_pipelines(
        Device device,
        std::span<VkComputePipelineCreateInfo const> create_infos,
        std::optional<VkPipelineCache> pipeline_cache = {},
        std::optional<Allocation_callbacks> vulkan_allocator = {},
        std::pmr::polymorphic_allocator<VkPipeline> polymorphic_allocator = {}
    ) noexcept;

    export std::pmr::vector<VkPipeline> create_graphics_pipelines(
        Device device,
        std::span<VkGraphicsPipelineCreateInfo const> create_infos,
        std::optional<VkPipelineCache> pipeline_cache = {},
        std::optional<Allocation_callbacks> vulkan_allocator = {},
        std::pmr::polymorphic_allocator<VkPipeline> polymorphic_allocator = {}
    ) noexcept;

    export void destroy_pipeline(
        Device device,
        VkPipeline pipeline,
        std::optional<Allocation_callbacks> vulkan_allocator = {}
    ) noexcept;
}
