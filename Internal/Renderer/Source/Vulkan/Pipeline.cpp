module maia.renderer.vulkan.pipeline;

import maia.renderer.vulkan.allocation_callbacks;
import maia.renderer.vulkan.check;
import maia.renderer.vulkan.device;

import <vulkan/vulkan.h>;

import <memory_resource>;
import <optional>;
import <span>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    std::pmr::vector<VkPipeline> create_compute_pipelines(
        Device const device,
        std::span<VkComputePipelineCreateInfo const> const create_infos,
        std::optional<VkPipelineCache> const pipeline_cache,
        std::optional<Allocation_callbacks> const vulkan_allocator,
        std::pmr::polymorphic_allocator<VkPipeline> const polymorphic_allocator
    ) noexcept
    {
        std::pmr::vector<VkPipeline> pipelines{polymorphic_allocator};
        pipelines.resize(create_infos.size());

        check_result(
            vkCreateComputePipelines(
                device.value,
                pipeline_cache.has_value() ? *pipeline_cache : VK_NULL_HANDLE,
                create_infos.size(),
                create_infos.data(),
                vulkan_allocator.has_value() ? &vulkan_allocator->value : nullptr,
                pipelines.data()
            )
        );

        return pipelines;
    }

    std::pmr::vector<VkPipeline> create_graphics_pipelines(
        Device const device,
        std::span<VkGraphicsPipelineCreateInfo const> const create_infos,
        std::optional<VkPipelineCache> const pipeline_cache,
        std::optional<Allocation_callbacks> const vulkan_allocator,
        std::pmr::polymorphic_allocator<VkPipeline> const polymorphic_allocator
    ) noexcept
    {
        std::pmr::vector<VkPipeline> pipelines{polymorphic_allocator};
        pipelines.resize(create_infos.size());

        check_result(
            vkCreateGraphicsPipelines(
                device.value,
                pipeline_cache.has_value() ? *pipeline_cache : VK_NULL_HANDLE,
                create_infos.size(),
                create_infos.data(),
                vulkan_allocator.has_value() ? &vulkan_allocator->value : nullptr,
                pipelines.data()
            )
        );

        return pipelines;
    }

    void destroy_pipeline(
        Device const device,
        VkPipeline const pipeline,
        std::optional<Allocation_callbacks> const vulkan_allocator
    ) noexcept
    {
        vkDestroyPipeline(
            device.value,
            pipeline,
            vulkan_allocator.has_value() ? &vulkan_allocator->value : nullptr
        );
    }
}
