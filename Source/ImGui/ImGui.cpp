module mythology.imgui;

import maia.renderer.vulkan;

import <imgui.h>;

import <vulkan/vulkan.h>;

import <array>;
import <cassert>;
import <cstring>;
import <optional>;
import <span>;

using namespace Maia::Renderer::Vulkan;

namespace Mythology::ImGui
{
    void upload_texture_data(
        VkDevice const device,
        VkDeviceMemory const device_memory,
        VkImage const image,
        std::span<std::byte const> const source,
        std::uint32_t const source_row_pitch
    ) noexcept
    {
        assert((source.size_bytes() % source_row_pitch) == 0);

        VkSubresourceLayout const subresource_layout = get_subresource_layout(
            {device},
            {image},
            {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .arrayLayer = 0}
        );
        assert(source.size_bytes() < subresource_layout.size);

        Mapped_memory const mapped_memory
        {
            {device},
            {device_memory},
            subresource_layout.offset,
            subresource_layout.size
        };

        std::uint32_t const num_rows_to_upload = source.size_bytes() / source_row_pitch;

        for (std::uint32_t row_index = 0; row_index < num_rows_to_upload; ++row_index)
        {
            std::uint64_t const row_destination_offset =
                row_index * subresource_layout.rowPitch;

            std::byte* const row_destination = 
                static_cast<std::byte*>(mapped_memory.data()) + row_destination_offset;

            std::uint64_t const row_source_offset =
                row_index * source_row_pitch;

            std::byte const* const row_source = source.data() + row_source_offset;
            
            std::memcpy(row_destination, row_source, source_row_pitch);
        }
    }

    void create_fonts_texture(
        Physical_device_memory_properties const& physical_device_memory_properties,
        VkDevice const device,
        VkDescriptorSet const descriptor_set,
        VkCommandBuffer const command_buffer
        // TODO allocator
        ) noexcept
    {
        unsigned char* pixels = nullptr;
        int width = 0;
        int height = 0;
        {
            ImGuiIO& io = ::ImGui::GetIO();

            io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        }

        VkResult err;

        Image font_image;
        {
            font_image = create_image(
                {device},
                {},
                VK_IMAGE_TYPE_2D,
                VK_FORMAT_R8G8B8A8_UNORM,
                {static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height), 1},
                Mip_level_count{1},
                Array_layer_count{1},
                VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_SAMPLE_COUNT_1_BIT,
                {},
                VK_IMAGE_TILING_OPTIMAL,
                VK_SHARING_MODE_EXCLUSIVE
            );

            Memory_requirements const memory_requirements = 
                get_memory_requirements({device}, font_image);

            Memory_type_bits const memory_type_bits = get_memory_type_bits(memory_requirements);

            std::optional<Memory_type_index> const memory_type_index = 
                find_memory_type(physical_device_memory_properties, memory_type_bits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            
            // allocate_memory({device}, )
            // TODO allocate and bind

            /*VkMemoryAllocateInfo alloc_info = {};
            alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            alloc_info.allocationSize = req.size;
            alloc_info.memoryTypeIndex = ;
            err = vkAllocateMemory(v->Device, &alloc_info, v->Allocator, &g_FontMemory);
            check_vk_result(err);

            err = vkBindImageMemory(v->Device, g_FontImage, g_FontMemory, 0);
            check_vk_result(err);*/
        }

        Image_view font_image_view;
        {
            font_image_view = create_image_view(
                {device},
                {},
                font_image,
                VK_IMAGE_VIEW_TYPE_2D,
                VK_FORMAT_R8G8B8A8_UNORM,
                {},
                VkImageSubresourceRange
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                },
                {}
            );
        }

        {
            std::array<VkDescriptorImageInfo, 1> const descriptor_image_infos
            {
                {
                    {
                        .sampler = VK_NULL_HANDLE,
                        .imageView = font_image_view.value,
                        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                    }
                }
            };

            VkWriteDescriptorSet const write_descriptor_set
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = descriptor_set,
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = descriptor_image_infos.size(),
                .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .pImageInfo = descriptor_image_infos.data(),
                .pBufferInfo = nullptr,
                .pTexelBufferView = nullptr,
            };

            vkUpdateDescriptorSets(device, 1, &write_descriptor_set, 0, nullptr);
        }

        // TODO create and bind memory somehow
        // TODO copy to image

        /*size_t const upload_size = width * height * 4 * sizeof(char);
        {

            VkBufferCreateInfo buffer_info = {};
            buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            buffer_info.size = upload_size;
            buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            err = vkCreateBuffer(v->Device, &buffer_info, v->Allocator, &g_UploadBuffer);
            check_vk_result(err);
            VkMemoryRequirements req;
            vkGetBufferMemoryRequirements(v->Device, g_UploadBuffer, &req);
            g_BufferMemoryAlignment = (g_BufferMemoryAlignment > req.alignment) ? g_BufferMemoryAlignment : req.alignment;
            VkMemoryAllocateInfo alloc_info = {};
            alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            alloc_info.allocationSize = req.size;
            alloc_info.memoryTypeIndex = ImGui_ImplVulkan_MemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, req.memoryTypeBits);
            err = vkAllocateMemory(v->Device, &alloc_info, v->Allocator, &g_UploadBufferMemory);
            check_vk_result(err);
            err = vkBindBufferMemory(v->Device, g_UploadBuffer, g_UploadBufferMemory, 0);
            check_vk_result(err);
        }

        // Upload to Buffer:
        {
            char* map = NULL;
            err = vkMapMemory(v->Device, g_UploadBufferMemory, 0, upload_size, 0, (void**)(&map));
            check_vk_result(err);
            memcpy(map, pixels, upload_size);
            VkMappedMemoryRange range[1] = {};
            range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            range[0].memory = g_UploadBufferMemory;
            range[0].size = upload_size;
            err = vkFlushMappedMemoryRanges(v->Device, 1, range);
            check_vk_result(err);
            vkUnmapMemory(v->Device, g_UploadBufferMemory);
        }

        // Copy to Image:
        {
            VkImageMemoryBarrier copy_barrier[1] = {};
            copy_barrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            copy_barrier[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            copy_barrier[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            copy_barrier[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            copy_barrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            copy_barrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            copy_barrier[0].image = g_FontImage;
            copy_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copy_barrier[0].subresourceRange.levelCount = 1;
            copy_barrier[0].subresourceRange.layerCount = 1;
            vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, copy_barrier);

            VkBufferImageCopy region = {};
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.layerCount = 1;
            region.imageExtent.width = width;
            region.imageExtent.height = height;
            region.imageExtent.depth = 1;
            vkCmdCopyBufferToImage(command_buffer, g_UploadBuffer, g_FontImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

            VkImageMemoryBarrier use_barrier[1] = {};
            use_barrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            use_barrier[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            use_barrier[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            use_barrier[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            use_barrier[0].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            use_barrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            use_barrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            use_barrier[0].image = g_FontImage;
            use_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            use_barrier[0].subresourceRange.levelCount = 1;
            use_barrier[0].subresourceRange.layerCount = 1;
            vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, use_barrier);
        }*/

        {
            ImGuiIO& io = ::ImGui::GetIO();
            io.Fonts->TexID = reinterpret_cast<ImTextureID>(font_image.value);
        }
    }

    void create_device_objects(
        VkDevice const device,
        VkRenderPass const render_pass,
        std::uint32_t const subpass_index,
        VkShaderModule const vertex_shader_module,
        VkShaderModule const fragment_shader_module,
        VkDescriptorPool const descriptor_pool,
        VkPipelineCache const pipeline_cache
        // TODO allocator
    ) noexcept
    {
        VkResult err;

        VkSampler font_sampler = {};
        {
            VkSamplerCreateInfo const info
            {
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .magFilter = VK_FILTER_LINEAR,
                .minFilter = VK_FILTER_LINEAR,
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .maxAnisotropy = 1.0f,
                .minLod = -1000,
                .maxLod = 1000,
            };
            
            check_result(
                vkCreateSampler(device, &info, nullptr, &font_sampler));
        }

        VkDescriptorSetLayout descriptor_set_layout = {};
        {
            VkDescriptorSetLayoutBinding const binding
            {
                .binding = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                .pImmutableSamplers = &font_sampler,
            };
            
            VkDescriptorSetLayoutCreateInfo const info
            {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .bindingCount = 1,
                .pBindings = &binding,
            };
            
            check_result(
                vkCreateDescriptorSetLayout(device, &info, nullptr, &descriptor_set_layout));
        }

        VkDescriptorSet descriptor_set = {};
        {
            VkDescriptorSetAllocateInfo const allocate_info
            {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .pNext = nullptr,
                .descriptorPool = descriptor_pool,
                .descriptorSetCount = 1,
                .pSetLayouts = &descriptor_set_layout,
            };
            
            check_result(
                vkAllocateDescriptorSets(device, &allocate_info, &descriptor_set));
        }

        VkPipelineLayout pipeline_layout = {};
        {
            VkPushConstantRange constexpr push_constant_range
            {
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .offset = sizeof(float) * 0,
                .size = sizeof(float) * 4,
            };

            VkPipelineLayoutCreateInfo const layout_info
            {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .setLayoutCount = 1,
                .pSetLayouts = &descriptor_set_layout,
                .pushConstantRangeCount = 1,
                .pPushConstantRanges = &push_constant_range,
            };

            check_result(
                vkCreatePipelineLayout(device, &layout_info, nullptr, &pipeline_layout));
        }

        std::array<VkPipelineShaderStageCreateInfo, 2> const shader_stages
        {
            {
                {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = {},
                    .stage = VK_SHADER_STAGE_VERTEX_BIT,
                    .module = vertex_shader_module,
                    .pName = "main",
                    .pSpecializationInfo = nullptr,
                },
                {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = {},
                    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .module = fragment_shader_module,
                    .pName = "main",
                    .pSpecializationInfo = nullptr,
                }
            }
        };

        VkVertexInputBindingDescription const binding_description
        {
            .binding = 0,
            .stride = sizeof(ImDrawVert),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };

        std::array<VkVertexInputAttributeDescription, 3> const attribute_descriptions
        {
            {
                {
                    .location = 0,
                    .binding = binding_description.binding,
                    .format = VK_FORMAT_R32G32_SFLOAT,
                    .offset = IM_OFFSETOF(ImDrawVert, pos),
                },
                {
                    .location = 1,
                    .binding = binding_description.binding,
                    .format = VK_FORMAT_R32G32_SFLOAT,
                    .offset = IM_OFFSETOF(ImDrawVert, uv),
                },
                {
                    .location = 2,
                    .binding = binding_description.binding,
                    .format = VK_FORMAT_R8G8B8A8_UNORM,
                    .offset = IM_OFFSETOF(ImDrawVert, col),
                },
            }
        };

        VkPipelineVertexInputStateCreateInfo const vertex_info
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &binding_description,
            .vertexAttributeDescriptionCount = attribute_descriptions.size(),
            .pVertexAttributeDescriptions = attribute_descriptions.data(),
        };
        
        VkPipelineInputAssemblyStateCreateInfo constexpr input_assembly_info
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE,
        };

        VkPipelineViewportStateCreateInfo constexpr viewport_info
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .viewportCount = 1,
            .pViewports = nullptr,
            .scissorCount = 1,
            .pScissors = nullptr,
        };

        VkPipelineRasterizationStateCreateInfo constexpr rasterization_info
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_NONE,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = {},
            .depthBiasClamp = {},
            .depthBiasSlopeFactor = {},
            .lineWidth = 1.0f,
        };

        VkPipelineMultisampleStateCreateInfo constexpr multisample_state_info
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = {},
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE,
        };

        VkPipelineColorBlendAttachmentState constexpr color_attachment
        {
            .blendEnable = VK_TRUE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        };

        VkPipelineDepthStencilStateCreateInfo constexpr depth_stencil_info
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

        VkPipelineColorBlendStateCreateInfo const blend_state_info
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .logicOpEnable = VK_FALSE,
            .logicOp = {},
            .attachmentCount = 1,
            .pAttachments = &color_attachment,
            .blendConstants = {},
        };

        std::array<VkDynamicState, 2> constexpr dynamic_states
        {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo const dynamic_state
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .dynamicStateCount = dynamic_states.size(),
            .pDynamicStates = dynamic_states.data(),
        };

        VkGraphicsPipelineCreateInfo info
        {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .stageCount = shader_stages.size(),
            .pStages = shader_stages.data(),
            .pVertexInputState = &vertex_info,
            .pInputAssemblyState = &input_assembly_info,
            .pTessellationState = nullptr,
            .pViewportState = &viewport_info,
            .pRasterizationState = &rasterization_info,
            .pMultisampleState = &multisample_state_info,
            .pDepthStencilState = &depth_stencil_info,
            .pColorBlendState = &blend_state_info,
            .pDynamicState = &dynamic_state,
            .layout = pipeline_layout,
            .renderPass = render_pass,
            .subpass = subpass_index,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = 0,
        };

        VkPipeline pipeline = {};
        check_result(
            vkCreateGraphicsPipelines(device, pipeline_cache, 1, &info, nullptr, &pipeline));

        // TODO destroy shader modules
        //vkDestroyShaderModule(v->Device, vert_module, v->Allocator);
        //vkDestroyShaderModule(v->Device, frag_module, v->Allocator);
    }

    struct Buffer_range
    {
        VkBuffer buffer;
        VkDeviceSize offset;
        VkDeviceSize size;
    };

    void upload_vertex_and_index_buffers_data(
        void* const raw_vertex_destination,
        void* const raw_index_destination,
        ImDrawData const& draw_data
    ) noexcept
    {
        using Vertex_type = ImDrawVert;
        using Index_type = ImDrawIdx;

        Vertex_type* vertex_destination = static_cast<Vertex_type*>(raw_vertex_destination);
        Index_type* index_destination = static_cast<Index_type*>(raw_index_destination);

        for (int command_list_index = 0; command_list_index < draw_data.CmdListsCount; command_list_index++)
        {
            ImDrawList const& command_list = *draw_data.CmdLists[command_list_index];

            std::memcpy(
                vertex_destination,
                command_list.VtxBuffer.Data,
                command_list.VtxBuffer.Size * sizeof(Vertex_type)
            );

            std::memcpy(
                index_destination,
                command_list.IdxBuffer.Data,
                command_list.IdxBuffer.Size * sizeof(Index_type)
            );
            
            vertex_destination += command_list.VtxBuffer.Size;
            index_destination += command_list.IdxBuffer.Size;
        }
    }

    namespace
    {
        void setup_render_state(
            ImDrawData const& draw_data,
            VkCommandBuffer const command_buffer,
            VkPipeline const pipeline,
            VkPipelineLayout const pipeline_layout,
            VkDescriptorSet const descriptor_set,
            Buffer_range const vertex_buffer_range,
            Buffer_range const index_buffer_range,
            int const framebuffer_width,
            int const framebuffer_height
        ) noexcept
        {
            vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
                
            vkCmdBindDescriptorSets(
                command_buffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipeline_layout,
                0, 1, &descriptor_set,
                0, nullptr
            );

            {
                vkCmdBindVertexBuffers(
                    command_buffer,
                    0,
                    1,
                    &vertex_buffer_range.buffer,
                    &vertex_buffer_range.offset
                );

                VkIndexType constexpr index_type = 
                    sizeof(ImDrawIdx) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;

                vkCmdBindIndexBuffer(
                    command_buffer,
                    index_buffer_range.buffer,
                    index_buffer_range.offset,
                    index_type
                );
            }

            {
                VkViewport const viewport
                {
                    .x = 0,
                    .y = 0,
                    .width = static_cast<float>(framebuffer_width),
                    .height = static_cast<float>(framebuffer_height),
                    .minDepth = 0.0f,
                    .maxDepth = 1.0f
                };

                vkCmdSetViewport(command_buffer, 0, 1, &viewport);
            }

            // Setup scale and translation:
            // Our visible imgui space lies from draw_data.DisplayPps (top left) to draw_data.DisplayPos+data_data.DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
            {
                std::array<float, 2> const scale
                {
                    2.0f / draw_data.DisplaySize.x,
                    2.0f / draw_data.DisplaySize.y
                };

                std::uint32_t constexpr scale_size_bytes = sizeof(float) * scale.size();

                vkCmdPushConstants(
                    command_buffer,
                    pipeline_layout,
                    VK_SHADER_STAGE_VERTEX_BIT,
                    sizeof(float) * 0,
                    scale_size_bytes,
                    scale.data()
                );

                std::array<float, 2> const translation
                {
                    -1.0f - draw_data.DisplayPos.x * scale[0],
                    -1.0f - draw_data.DisplayPos.y * scale[1]
                };

                std::uint32_t constexpr translate_offset_bytes = scale_size_bytes;

                vkCmdPushConstants(
                    command_buffer,
                    pipeline_layout,
                    VK_SHADER_STAGE_VERTEX_BIT,
                    translate_offset_bytes,
                    sizeof(float) * translation.size(),
                    translation.data()
                );
            }
        }
    }

    // TODO create fonts texture and upload data to it
    // TODO create pipeline, pipeline layout, descriptor set

    void render(
        ImDrawData const& draw_data,
        VkCommandBuffer const command_buffer,
        VkPipeline const pipeline,
        VkPipelineLayout const pipeline_layout,
        VkDescriptorSet const descriptor_set,
        Buffer_range const vertex_buffer_range,
        Buffer_range const index_buffer_range
    ) noexcept
    {
        assert((draw_data.TotalVtxCount * sizeof(ImDrawVert)) <= vertex_buffer_range.size);
        assert((draw_data.TotalIdxCount * sizeof(ImDrawIdx)) <= index_buffer_range.size);

        int const framebuffer_width = static_cast<int>(draw_data.DisplaySize.x * draw_data.FramebufferScale.x);
        int const framebuffer_height = static_cast<int>(draw_data.DisplaySize.y * draw_data.FramebufferScale.y);
        
        if (framebuffer_width <= 0 || framebuffer_height <= 0 || draw_data.TotalVtxCount == 0)
            return;

        setup_render_state(
            draw_data,
            command_buffer,
            pipeline,
            pipeline_layout,
            descriptor_set,
            vertex_buffer_range,
            index_buffer_range,
            framebuffer_width,
            framebuffer_height
        );
        
        ImVec2 const clip_offset = draw_data.DisplayPos;
        ImVec2 const clip_scale = draw_data.FramebufferScale;

        int vertex_offset = 0;
        int index_offset = 0;
        
        for (int command_list_index = 0; command_list_index < draw_data.CmdListsCount; command_list_index++)
        {
            ImDrawList const& command_list = *draw_data.CmdLists[command_list_index];

            for (int command_index = 0; command_index < command_list.CmdBuffer.Size; command_index++)
            {
                ImDrawCmd const& draw_command = command_list.CmdBuffer[command_index];
                
                if (draw_command.UserCallback != nullptr)
                {
                    // User callback, registered via ImDrawList::AddCallback()
                    // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                    if (draw_command.UserCallback == ImDrawCallback_ResetRenderState)
                    {
                        setup_render_state(
                            draw_data,
                            command_buffer,
                            pipeline,
                            pipeline_layout,
                            descriptor_set,
                            vertex_buffer_range,
                            index_buffer_range,
                            framebuffer_width,
                            framebuffer_height
                        );
                    }
                    else
                    {
                        draw_command.UserCallback(&command_list, &draw_command);
                    }
                }
                else
                {
                    // Project scissor/clipping rectangles into framebuffer space
                    ImVec4 const clip_rect
                    {
                        (draw_command.ClipRect.x - clip_offset.x) * clip_scale.x,
                        (draw_command.ClipRect.y - clip_offset.y) * clip_scale.y,
                        (draw_command.ClipRect.z - clip_offset.x) * clip_scale.x,
                        (draw_command.ClipRect.w - clip_offset.y) * clip_scale.y
                    };
                    
                    if (clip_rect.x < framebuffer_width && clip_rect.y < framebuffer_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
                    {
                        {
                            assert((clip_rect.x >= 0.0f && clip_rect.y >= 0.0f) && "Negative offsets are illegal");

                            VkRect2D const scissor
                            {
                                .offset = 
                                {
                                    .x = static_cast<int32_t>(clip_rect.x),
                                    .y = static_cast<int32_t>(clip_rect.y)
                                },
                                .extent =
                                {
                                    .width = static_cast<uint32_t>(clip_rect.z - clip_rect.x),
                                    .height = static_cast<uint32_t>(clip_rect.w - clip_rect.y)
                                }
                            };

                            vkCmdSetScissor(command_buffer, 0, 1, &scissor);
                        }

                        std::uint32_t constexpr first_instance = 0;
                        std::uint32_t constexpr instance_count = 1;

                        vkCmdDrawIndexed(
                            command_buffer,
                            draw_command.ElemCount,
                            instance_count,
                            draw_command.IdxOffset + index_offset,
                            draw_command.VtxOffset + vertex_offset,
                            first_instance
                        );
                    }
                }
            }

            vertex_offset += command_list.VtxBuffer.Size;
            index_offset += command_list.IdxBuffer.Size;
        }
    }
}
