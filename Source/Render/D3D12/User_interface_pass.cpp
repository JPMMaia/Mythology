#include "User_interface_pass.hpp"

#include <array>

#include <d3dx12.h>

#include <Maia/Renderer/D3D12/Descriptor.hpp>
#include <Maia/Renderer/D3D12/Descriptor_heap_view.hpp>
#include <Maia/Renderer/D3D12/Heap_view.hpp>
#include <Maia/Renderer/D3D12/Static_samplers.hpp>
#include <Maia/Renderer/D3D12/Utilities/Mapped_memory.hpp>

using namespace Maia::Renderer::D3D12;

namespace Maia::Mythology
{
	namespace
	{
		ImGuiContext* create_imgui_context()
		{
			IMGUI_CHECKVERSION();
			ImGuiContext* const context = ImGui::CreateContext();
			ImGui::StyleColorsDark();
			return context;
		}

		Maia::Renderer::D3D12::Sampled_image_2d create_font_image(
			ID3D12Device& device, 
			Non_rt_ds_image_heap_view const heap_view,
			ID3D12GraphicsCommandList& command_list,
			Upload_buffer_view const upload_buffer_view
		)
		{
			ImGuiIO& io = ImGui::GetIO();

			unsigned char* pixels;
			int width, height;
			io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);

			Sampled_image_2d font_image
			{
				device,
				heap_view,
				DXGI_FORMAT_R8_UNORM,
				{ Image_width{ static_cast<UINT64>(width) }, Image_height{ static_cast<UINT>(height) } },
				Array_slices{ 1 }, Mip_levels{ 1 }
			};

			// Upload image data
			{
				Mapped_memory upload_buffer_mapped_memory{ *upload_buffer_view.buffer.value, 0, {} };

				UINT const subresource_index = 0;

				D3D12_PLACED_SUBRESOURCE_FOOTPRINT image_layout;
				{
					D3D12_RESOURCE_DESC const image_description = font_image.value->GetDesc();

					UINT num_rows;
					UINT64 row_size;
					UINT64 total_size;
					device.GetCopyableFootprints(
						&image_description,
						subresource_index, 1,
						0,
						&image_layout,
						&num_rows,
						&row_size,
						&total_size
					);
				}

				for (UINT y = 0; y < image_layout.Footprint.Height; y++)
				{
					SIZE_T const upload_buffer_offset = upload_buffer_view.offset.value + image_layout.Offset + static_cast<UINT64>(y) * static_cast<UINT64>(image_layout.Footprint.RowPitch);
					unsigned char const* const source_data = &pixels[y * image_layout.Footprint.Width];
					std::size_t const data_size = sizeof(DWORD) * image_layout.Footprint.Width;

					upload_buffer_mapped_memory.write(source_data, data_size, upload_buffer_offset);
				}

				{
					CD3DX12_TEXTURE_COPY_LOCATION const destination{ font_image.value.get(), subresource_index };

					CD3DX12_TEXTURE_COPY_LOCATION const source{ upload_buffer_view.buffer.value.get(), image_layout };

					command_list.CopyTextureRegion(
						&destination,
						0, 0, 0,
						&source,
						nullptr
					);
				}
			}

			// Issue barrier from copy_dest to pixel shader
			{
				CD3DX12_RESOURCE_BARRIER const resource_barrier =
					CD3DX12_RESOURCE_BARRIER::Transition(
						font_image.value.get(),
						D3D12_RESOURCE_STATE_COPY_DEST,
						D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
					);

				command_list.ResourceBarrier(1, &resource_barrier);
			}

			return font_image;
		}

		Maia::Renderer::D3D12::Root_signature create_root_signature(
			ID3D12Device& device
		) noexcept
		{
			std::array<Descriptor_range, 1> constexpr descriptor_ranges
			{
				Shader_resource_view_descriptor_range{ Descriptor_range_size{ 1 }, Shader_register{ 0 }, Shader_register_space{ 1 }, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, Descriptor_table_offset_in_descriptors{ 0 } }
			};

			std::array<Root_signature_parameter, 2> constexpr root_parameters
			{
				Constant_buffer_view_root_descriptor_root_signature_parameter{ Shader_register{ 0 }, Shader_register_space{1}, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_VERTEX },
				Cbv_srv_uav_descriptor_table_root_signature_parameter{ descriptor_ranges, D3D12_SHADER_VISIBILITY_PIXEL }
			};

			std::array<Static_sampler, 6> const static_samplers{ create_static_samplers() };

			return Root_signature
			{
				device,
				root_parameters,
				static_samplers,
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
			};
		}

		Cbv_srv_uav_descriptor_table create_texture_descriptor_table(
			ID3D12Device& device,
			Cbv_srv_uav_descriptor_heap_view const descriptor_heap_view,
			Shader_resource_image_2d_view const font_texture_view
		) noexcept
		{
			return Cbv_srv_uav_descriptor_table
			{
				device,
				Descriptor_table_base_cpu_descriptor{ descriptor_heap_view.descriptor_heap.value->GetCPUDescriptorHandleForHeapStart() },
				Descriptor_table_base_gpu_descriptor{ descriptor_heap_view.descriptor_heap.value->GetGPUDescriptorHandleForHeapStart() },
				Cbv_srv_uav_descriptor_handle_increment_size{ device },
				std::make_tuple(
					Shader_resource_2d_descriptor{ font_texture_view, DXGI_FORMAT_R8_UNORM }
				)
			};
		}
	}

	User_interface_pass::User_interface_pass(
		ID3D12Device& device, 
		ID3D12GraphicsCommandList& command_list,
		Maia::Renderer::D3D12::Upload_buffer_view const upload_buffer_view,
		std::pair<float, float> const display_size
	) noexcept :
		m_context { create_imgui_context() },
		m_image_heap{ device, Heap_size{ 3 * D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT } },
		m_font_texture{ create_font_image(device, { m_image_heap, Heap_offset{ 0 }, Heap_size{ 3 * D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT }  }, command_list, upload_buffer_view) },
		m_descriptor_heap{ device, Descriptor_heap_size{ 1 } },
		m_root_signature{ create_root_signature(device) },
		m_texture_descriptor_table{ create_texture_descriptor_table(device, { m_descriptor_heap, Descriptor_heap_offset{0}, Descriptor_heap_size{1} }, { m_font_texture, First_mip_level{ 0 }, Mip_levels{ 1 } }) },
		m_geometry_buffer_heap{ device, Heap_size{ 3 * D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT } },
		m_geometry_buffer{ device, { m_geometry_buffer_heap, 0, 3 * D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT } }
	{
		{
			ImGuiIO& io = ImGui::GetIO();

			// Needs to be done when window size changes
			io.DisplaySize.x = display_size.first;
			io.DisplaySize.y = display_size.second;
		}
	}


	namespace
	{
		void upload_vertex_buffer_data(
			Dynamic_vertex_buffer_view const vertex_buffer_view,
			ImDrawData const& draw_data
		) noexcept
		{
			assert(draw_data.TotalVtxCount * sizeof(ImDrawVert) < vertex_buffer_view.size.value);

			Mapped_memory mapped_memory{ *vertex_buffer_view.buffer.value, 0, {} };

			{
				Buffer_offset offset = vertex_buffer_view.offset;

				for (int command_list_index = 0; command_list_index < draw_data.CmdListsCount; ++command_list_index)
				{
					ImDrawList const& command_list = *draw_data.CmdLists[command_list_index];

					std::size_t const size = command_list.VtxBuffer.Size * sizeof(ImDrawVert);
					mapped_memory.write(command_list.VtxBuffer.Data, size, offset.value);

					offset.value += size;
				}
			}
		}

		void upload_index_buffer_data(
			Dynamic_index_buffer_view const index_buffer_view,
			ImDrawData const& draw_data
		) noexcept
		{
			assert(draw_data.TotalIdxCount * sizeof(ImDrawIdx) < index_buffer_view.size.value);

			Mapped_memory mapped_memory{ *index_buffer_view.buffer.value, 0, {} };

			{
				Buffer_offset offset = index_buffer_view.offset;

				for (int command_list_index = 0; command_list_index < draw_data.CmdListsCount; ++command_list_index)
				{
					ImDrawList const& command_list = *draw_data.CmdLists[command_list_index];

					std::size_t const size = command_list.IdxBuffer.Size * sizeof(ImDrawIdx);
					mapped_memory.write(command_list.IdxBuffer.Data, size, offset.value);

					offset.value += size;
				}
			}
		}
	}

	void User_interface_pass::upload_user_interface_data(
		Frame_index const frame_index
	) noexcept
	{
		// TODO move this to another place
		{
			ImGui::NewFrame();
			// TODO Draw
		}

		{
			// TODO upload pass data
		}

		{
			ImGui::Render();
			ImDrawData const& draw_data = *ImGui::GetDrawData();

			{
				Dynamic_geometry_buffer_view const geometry_buffer_view{ m_geometry_buffer, Buffer_offset{ frame_index.value * static_cast<UINT64>(D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT) }, Buffer_size{ static_cast<UINT64>(D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT) } };

				{
					std::uint64_t const required_vertex_buffer_size = static_cast<std::size_t>(draw_data.TotalVtxCount) * sizeof(ImDrawVert);
					std::uint64_t const required_index_buffer_size = static_cast<std::size_t>(draw_data.TotalIdxCount) * sizeof(ImDrawIdx);
					assert(required_vertex_buffer_size + required_index_buffer_size < geometry_buffer_view.size.value);

					{
						Dynamic_vertex_buffer_view const vertex_buffer_view{ geometry_buffer_view.buffer, geometry_buffer_view.offset, Buffer_size{ required_vertex_buffer_size } };
						upload_vertex_buffer_data(vertex_buffer_view, draw_data);
					}
					
					{
						Dynamic_index_buffer_view const index_buffer_view{ geometry_buffer_view.buffer, Buffer_offset{ geometry_buffer_view.offset.value + required_vertex_buffer_size }, Buffer_size{ required_index_buffer_size } };
						upload_index_buffer_data(index_buffer_view, draw_data);
					}
				}				
			}
		}
	}

	namespace
	{
		void reset_render_state(
			ID3D12GraphicsCommandList& command_list,
			ID3D12PipelineState& pipeline_state,
			Root_signature const& root_signature,
			Constant_buffer_view const pass_buffer_view,
			std::pair<FLOAT, FLOAT> const viewport_size,
			Dynamic_vertex_buffer_view const vertex_buffer_view,
			Dynamic_index_buffer_view const index_buffer_view
		) noexcept
		{
			command_list.SetPipelineState(&pipeline_state);

			{
				command_list.SetGraphicsRootSignature(root_signature.value.get());

				{
					// TODO
					D3D12_GPU_VIRTUAL_ADDRESS const buffer_location = pass_buffer_view.buffer.value->GetGPUVirtualAddress() + pass_buffer_view.offset.value;
					command_list.SetGraphicsRootConstantBufferView(0, buffer_location);
				}
			}

			{
				{
					UINT64 constexpr stride = sizeof(ImDrawVert);

					D3D12_VERTEX_BUFFER_VIEW view;
					view.BufferLocation = vertex_buffer_view.buffer.value->GetGPUVirtualAddress() + vertex_buffer_view.offset.value;
					view.SizeInBytes = static_cast<UINT>(vertex_buffer_view.size.value);
					view.StrideInBytes = stride;

					command_list.IASetVertexBuffers(0, 1, &view);
				}

				{
					DXGI_FORMAT constexpr format = sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

					D3D12_INDEX_BUFFER_VIEW view;
					view.BufferLocation = index_buffer_view.buffer.value->GetGPUVirtualAddress() + index_buffer_view.offset.value;
					view.SizeInBytes = static_cast<UINT>(index_buffer_view.size.value);
					view.Format = format;

					command_list.IASetIndexBuffer(&view);
				}

				command_list.IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			}

			{
				D3D12_VIEWPORT viewport;
				viewport.TopLeftX = 0.0f;
				viewport.TopLeftY = 0.0f;
				viewport.Width = viewport_size.first;
				viewport.Height = viewport_size.second;
				viewport.MinDepth = 0.0f;
				viewport.MaxDepth = 1.0f;

				command_list.RSSetViewports(1, &viewport);
			}

			{
				FLOAT const blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
				command_list.OMSetBlendFactor(blend_factor);
			}
		}
	}

	void User_interface_pass::execute_user_interface_pass(
		ID3D12GraphicsCommandList& command_list,
		Constant_buffer_view const pass_buffer_view
	) noexcept
	{
		// reset_render_state(command_list, pipeline_state, root_signature, pass_buffer_view, { draw_data.DisplaySize.x, draw_data.DisplaySize.y }, {}, {});

		{
			ImDrawData const& draw_data = *ImGui::GetDrawData();

			int base_vertex_location = 0;
			int start_index_location = 0;
			ImVec2 const display_position = draw_data.DisplayPos;
			
			for (int draw_list_index = 0; draw_list_index < draw_data.CmdListsCount; ++draw_list_index)
			{
				ImDrawList const& draw_list = *draw_data.CmdLists[draw_list_index];

				for (int command_index = 0; command_index < draw_list.CmdBuffer.Size; command_index++)
				{
					ImDrawCmd const& draw_command = draw_list.CmdBuffer[command_index];			

					if (draw_command.UserCallback != NULL)
					{
						if (draw_command.UserCallback == ImDrawCallback_ResetRenderState)
						{
							//reset_render_state(command_list, pipeline_state, root_signature, pass_buffer_view, { draw_data.DisplaySize.x, draw_data.DisplaySize.y }, {}, {});
						}
						else
						{
							draw_command.UserCallback(&draw_list, &draw_command);
						}
					}
					else
					{
						{
							D3D12_RECT const scissor_rect
							{
								static_cast<LONG>(draw_command.ClipRect.x - display_position.x),
								static_cast<LONG>(draw_command.ClipRect.y - display_position.y),
								static_cast<LONG>(draw_command.ClipRect.z - display_position.x),
								static_cast<LONG>(draw_command.ClipRect.w - display_position.y)
							};

							command_list.RSSetScissorRects(1, &scissor_rect);
						}

						command_list.SetGraphicsRootDescriptorTable(1, m_texture_descriptor_table.base_gpu_descriptor.value);
						
						command_list.DrawIndexedInstanced(draw_command.ElemCount, 1, start_index_location, base_vertex_location, 0);
					}
						
			
					start_index_location += draw_command.ElemCount;
				}

				base_vertex_location += draw_list.VtxBuffer.Size;
			}
		}
	}
}