module;

#include <vulkan/vulkan.h>

#include <array>
#include <memory_resource>
#include <span>
#include <vector>

export module maia.renderer.vulkan.pipeline;
namespace Maia::Renderer::Vulkan
{
    export VkPipeline create_compute_pipeline(
        VkDevice device,
        VkComputePipelineCreateInfo const& create_info,
        VkPipelineCache pipeline_cache = VK_NULL_HANDLE,
        VkAllocationCallbacks const* vulkan_allocator = nullptr
    ) noexcept;

    export std::pmr::vector<VkPipeline> create_compute_pipelines(
        VkDevice device,
        std::span<VkComputePipelineCreateInfo const> create_infos,
        VkPipelineCache pipeline_cache = VK_NULL_HANDLE,
        VkAllocationCallbacks const* vulkan_allocator = nullptr,
        std::pmr::polymorphic_allocator<VkPipeline> const& vector_allocator = {}
    ) noexcept;

    export VkPipeline create_graphics_pipeline(
        VkDevice device,
        VkGraphicsPipelineCreateInfo const& create_info,
        VkPipelineCache pipeline_cache = VK_NULL_HANDLE,
        VkAllocationCallbacks const* vulkan_allocator = nullptr
    ) noexcept;

    export std::pmr::vector<VkPipeline> create_graphics_pipelines(
        VkDevice device,
        std::span<VkGraphicsPipelineCreateInfo const> create_infos,
        VkPipelineCache pipeline_cache = VK_NULL_HANDLE,
        VkAllocationCallbacks const* vulkan_allocator = nullptr,
        std::pmr::polymorphic_allocator<VkPipeline> vector_allocator = {}
    ) noexcept;

    export void destroy_pipeline(
        VkDevice device,
        VkPipeline pipeline,
        VkAllocationCallbacks const* vulkan_allocator = nullptr
    ) noexcept;


    export std::array<VkPipelineShaderStageCreateInfo, 2> constexpr create_shader_stages_create_info(
        VkShaderModule const vertex_shader,
        VkShaderModule const fragment_shader) noexcept
    {
        return 
        {
            VkPipelineShaderStageCreateInfo
            {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .stage = VK_SHADER_STAGE_VERTEX_BIT,
                .module = vertex_shader,
                .pName = "main",
                .pSpecializationInfo = nullptr
            },
            VkPipelineShaderStageCreateInfo
            {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                .module = fragment_shader,
                .pName = "main",
                .pSpecializationInfo = nullptr
            }
        };
    }

    export VkPipelineVertexInputStateCreateInfo constexpr create_vertex_input_state_create_info() noexcept
    {
        return
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .vertexBindingDescriptionCount = 0,
            .pVertexBindingDescriptions = nullptr,
            .vertexAttributeDescriptionCount = 0,
            .pVertexAttributeDescriptions = nullptr,
        };
    }

    export VkPipelineInputAssemblyStateCreateInfo constexpr create_input_assembly_state_create_info(
        VkPrimitiveTopology const primitive_topology) noexcept
    {
        return 
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .topology = primitive_topology,
            .primitiveRestartEnable = VK_FALSE,
        };
    }

    export VkPipelineViewportStateCreateInfo constexpr create_viewport_state_create_info(
        std::uint32_t const viewport_count,
        std::uint32_t const scissor_count) noexcept
    {
        return
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .viewportCount = viewport_count,
            .pViewports = nullptr,
            .scissorCount = scissor_count,
            .pScissors = nullptr,
        };
    }

    export VkPipelineMultisampleStateCreateInfo create_disabled_multisample_state_create_info() noexcept
    {
        static std::uint32_t constexpr sample_mask = 0xFFFFFFFF;

        return
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = {},
            .pSampleMask = &sample_mask,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE,
        };
    }

    export VkPipelineDepthStencilStateCreateInfo constexpr create_disabled_depth_stencil_state_create_info() noexcept
    {
        return
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .depthTestEnable = VK_FALSE,
            .depthWriteEnable = VK_FALSE,
            .depthCompareOp = {},
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE,
            .front = {},
            .back = {},
            .minDepthBounds = {},
            .maxDepthBounds = {},
        };
    }

    export VkPipelineDynamicStateCreateInfo create_viewport_scissor_dynamic_state_create_info() noexcept
    {
        static std::array<VkDynamicState, 2> constexpr dynamic_state
        {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        return
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .dynamicStateCount = dynamic_state.size(),
            .pDynamicStates = dynamic_state.data(),
        };
    }

    export VkPipelineColorBlendAttachmentState constexpr create_disabled_color_blend_attachment_state() noexcept
    {
        return
        {
            .blendEnable = VK_FALSE,
            .srcColorBlendFactor = {},
            .dstColorBlendFactor = {},
            .colorBlendOp = {},
            .srcAlphaBlendFactor = {},
            .dstAlphaBlendFactor = {},
            .alphaBlendOp = {},
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        };
    }

    export VkPipelineColorBlendStateCreateInfo constexpr create_disabled_color_blend_state_create_info(
        std::span<VkPipelineColorBlendAttachmentState const> const color_blend_attachment_states) noexcept
    {
        return 
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .logicOpEnable = VK_FALSE,
            .logicOp = {},
            .attachmentCount = static_cast<std::uint32_t>(color_blend_attachment_states.size()),
            .pAttachments = color_blend_attachment_states.data(),
            .blendConstants = {1.0f, 1.0, 1.0f, 1.0},
        };
    }
}
