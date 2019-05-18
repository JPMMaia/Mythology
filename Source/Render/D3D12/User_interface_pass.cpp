#include "User_interface_pass.hpp"

#include <imgui.h>

#include <d3dx12.h>

#include <Maia/Renderer/D3D12/Heaps/Image_heap_view.hpp>
#include <Maia/Renderer/D3D12/Utilities/D3D12_utilities.hpp>
#include <Maia/Renderer/D3D12/Utilities/Mapped_memory.hpp>
#include <Maia/Renderer/D3D12/Utilities/Upload_buffer.hpp>
#include <Maia/Renderer/D3D12/Utilities/Sampled_image.hpp>

using namespace Maia::Renderer::D3D12;

namespace Maia::Mythology
{
	namespace
	{
		std::pair<Maia::Renderer::D3D12::Image_heap, Maia::Renderer::D3D12::Sampled_image> create_font_image(
			ID3D12Device& device, 
			ID3D12GraphicsCommandList& command_list,
			Upload_buffer_view const upload_buffer_view
		)
		{
			// TODO move from here
			{
				IMGUI_CHECKVERSION();
				ImGui::CreateContext();
				ImGui::StyleColorsDark();
			}

			ImGuiIO& io = ImGui::GetIO();

			unsigned char* pixels;
			int width, height;
			io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

			UINT64 const image_heap_size = align(static_cast<UINT64>(width) * static_cast<UINT64>(height) * 4, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
			Image_heap image_heap{ device, image_heap_size };

			Sampled_image font_image
			{
				device,
				{ image_heap, 0, image_heap_size },
				DXGI_FORMAT_R8G8B8A8_UNORM,
				{ static_cast<UINT64>(width), static_cast<UINT>(height) },
				1
			};

			// Upload image data
			{
				Mapped_memory upload_buffer_mapped_memory{ upload_buffer_view.resource(), 0, {} };
				UINT64 const upload_buffer_begin_offset = upload_buffer_view.offset();

				UINT const subresource_index = 0;

				D3D12_PLACED_SUBRESOURCE_FOOTPRINT image_layout;
				{
					ID3D12Resource& resource = *font_image;
					D3D12_RESOURCE_DESC const image_description = resource.GetDesc();

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
					SIZE_T const upload_buffer_offset = upload_buffer_begin_offset + image_layout.Offset + y * image_layout.Footprint.RowPitch;
					unsigned char const* const source_data = &pixels[y * image_layout.Footprint.Width];
					std::size_t const data_size = sizeof(DWORD) * image_layout.Footprint.Width;

					upload_buffer_mapped_memory.write(source_data, data_size, upload_buffer_offset);
				}

				{
					CD3DX12_TEXTURE_COPY_LOCATION const destination{ font_image.get(), subresource_index };

					CD3DX12_TEXTURE_COPY_LOCATION const source{ &upload_buffer_view.resource(), image_layout };

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
						font_image.get(),
						D3D12_RESOURCE_STATE_COPY_DEST,
						D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
					);

				command_list.ResourceBarrier(1, &resource_barrier);
			}

			return { image_heap, font_image };
		}
	}

	User_interface_pass::User_interface_pass(
		ID3D12Device& device, 
		ID3D12GraphicsCommandList& command_list,
		Maia::Renderer::D3D12::Upload_buffer_view const upload_buffer_view,
		std::pair<float, float> const display_size
	) noexcept :
		m_font_texture{ create_font_image(device, command_list, upload_buffer_view) }
	{
		{
			ImGuiIO& io = ImGui::GetIO();

			// Needs to be done when window size changes
			io.DisplaySize.x = display_size.first;
			io.DisplaySize.y = display_size.second;
		}

		// TODO create font texture view

		// TODO set texture id handle
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

	void User_interface_pass::execute_user_interface_pass() noexcept
	{
		// TODO set viewport
		// TODO set pipeline state, root signature, blend factor

		// TODO set scissor, bind texture, draw
	}
}