module;

#include <vulkan/vulkan.h>

#include <memory_resource>
#include <optional>
#include <span>
#include <vector>

module maia.renderer.vulkan.pipeline;

import maia.renderer.vulkan.check;
namespace Maia::Renderer::Vulkan
{
    VkPipeline create_compute_pipeline(
        VkDevice const device,
        VkComputePipelineCreateInfo const& create_info,
        VkPipelineCache const pipeline_cache,
        VkAllocationCallbacks const* const vulkan_allocator
    ) noexcept
    {
        VkPipeline pipeline{};

        check_result(
            vkCreateComputePipelines(
                device,
                pipeline_cache,
                1,
                &create_info,
                vulkan_allocator,
                &pipeline
            )
        );

        return pipeline;
    }

    std::pmr::vector<VkPipeline> create_compute_pipelines(
        VkDevice const device,
        std::span<VkComputePipelineCreateInfo const> const create_infos,
        VkPipelineCache const pipeline_cache,
        VkAllocationCallbacks const* const vulkan_allocator,
        std::pmr::polymorphic_allocator<VkPipeline> const& vector_allocator
    ) noexcept
    {
        std::pmr::vector<VkPipeline> pipelines{vector_allocator};
        pipelines.resize(create_infos.size());

        check_result(
            vkCreateComputePipelines(
                device,
                pipeline_cache,
                create_infos.size(),
                create_infos.data(),
                vulkan_allocator,
                pipelines.data()
            )
        );

        return pipelines;
    }

    VkPipeline create_graphics_pipeline(
        VkDevice const device,
        VkGraphicsPipelineCreateInfo const& create_info,
        VkPipelineCache const pipeline_cache,
        VkAllocationCallbacks const* const vulkan_allocator
    ) noexcept
    {
        VkPipeline pipeline{};

        check_result(
            vkCreateGraphicsPipelines(
                device,
                pipeline_cache,
                1,
                &create_info,
                vulkan_allocator,
                &pipeline
            )
        );

        return pipeline;
    }

    std::pmr::vector<VkPipeline> create_graphics_pipelines(
        VkDevice const device,
        std::span<VkGraphicsPipelineCreateInfo const> const create_infos,
        VkPipelineCache const pipeline_cache,
        VkAllocationCallbacks const* const vulkan_allocator,
        std::pmr::polymorphic_allocator<VkPipeline> const polymorphic_allocator
    ) noexcept
    {
        std::pmr::vector<VkPipeline> pipelines{polymorphic_allocator};
        pipelines.resize(create_infos.size());

        check_result(
            vkCreateGraphicsPipelines(
                device,
                pipeline_cache,
                create_infos.size(),
                create_infos.data(),
                vulkan_allocator,
                pipelines.data()
            )
        );

        return pipelines;
    }

    void destroy_pipeline(
        VkDevice const device,
        VkPipeline const pipeline,
        VkAllocationCallbacks const* const vulkan_allocator
    ) noexcept
    {
        vkDestroyPipeline(
            device,
            pipeline,
            vulkan_allocator
        );
    }
}
