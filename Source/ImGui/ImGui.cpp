module mythology.imgui;

import maia.renderer.vulkan;

import <imgui.h>;

import <vulkan/vulkan.h>;

import <array>;
import <cassert>;
import <cstring>;
import <span>;

using namespace Maia::Renderer::Vulkan;

namespace Mythology::ImGui
{
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
