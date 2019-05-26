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
					SIZE_T const upload_buffer_offset = upload_buffer_view.offset.value + image_layout.Offset + y * image_layout.Footprint.RowPitch;
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
		m_root_signature{ create_root_signature(device) }
	{
		{
			ImGuiIO& io = ImGui::GetIO();

			// Needs to be done when window size changes
			io.DisplaySize.x = display_size.first;
			io.DisplaySize.y = display_size.second;
		}

		// TODO create font texture view
		// TODO set texture id handle

		Cbv_srv_uav_descriptor_table descriptor_table
		{
			device,
			Descriptor_table_base_cpu_descriptor{ m_descriptor_heap.value->GetCPUDescriptorHandleForHeapStart() },
			Descriptor_table_base_gpu_descriptor{ m_descriptor_heap.value->GetGPUDescriptorHandleForHeapStart() },
			Cbv_srv_uav_descriptor_handle_increment_size{ device },
			std::make_tuple(
				Shader_resource_2d_descriptor{ { m_font_texture, First_mip_level{ 0 }, Mip_levels{ 1 } }, DXGI_FORMAT_R8_UNORM }
			)
		};
	}

	void User_interface_pass::upload_user_interface_data() noexcept
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
			ImDrawData const* const draw_data = ImGui::GetDrawData();

			// TODO upload data
		}
	}

	void User_interface_pass::execute_user_interface_pass(
		ID3D12GraphicsCommandList& command_list
	) noexcept
	{
		// TODO set pipeline state

		command_list.SetGraphicsRootSignature(m_root_signature.value.get());

		{
			ImDrawData const& draw_data = *ImGui::GetDrawData();

			D3D12_VIEWPORT viewport;
			viewport.TopLeftX = 0.0f;
			viewport.TopLeftY = 0.0f;
			viewport.Width = draw_data.DisplaySize.x;
			viewport.Height = draw_data.DisplaySize.y;
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;
			
			command_list.RSSetViewports(1, &viewport);
		}

		{
			FLOAT const blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
			command_list.OMSetBlendFactor(blend_factor);
		}

		{
			// TODO set pass data 
			//command_list.SetGraphicsRootConstantBufferView();
		}

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
					assert(draw_command.UserCallback == nullptr);

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

						command_list.SetGraphicsRootDescriptorTable(1, *(D3D12_GPU_DESCRIPTOR_HANDLE*)& draw_command.TextureId); // TODO
						
						command_list.DrawIndexedInstanced(draw_command.ElemCount, 1, start_index_location, base_vertex_location, 0);
					}
						
			
					start_index_location += draw_command.ElemCount;
				}

				base_vertex_location += draw_list.VtxBuffer.Size;
			}
		}
	}
}