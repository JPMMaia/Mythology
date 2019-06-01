#ifndef MAIA_MYTHOLOGY_USERINTERFACEPASS_H_INCLUDED
#define MAIA_MYTHOLOGY_USERINTERFACEPASS_H_INCLUDED

#include <utility>

#include <d3d12.h>

#include <winrt/base.h>

#include <imgui.h>

#include <Maia/Renderer/D3D12/Descriptor.hpp>
#include <Maia/Renderer/D3D12/Descriptor_heap.hpp>
#include <Maia/Renderer/D3D12/Heap.hpp>
#include <Maia/Renderer/D3D12/Resource.hpp>
#include <Maia/Renderer/D3D12/Resource_view.hpp>
#include <Maia/Renderer/D3D12/Root_signature.hpp>

namespace Maia::Mythology
{
	struct Frame_index
	{
		std::uint8_t value;
	};

	class User_interface_pass
	{
	public:

		User_interface_pass(
			ID3D12Device& device,
			ID3D12GraphicsCommandList& command_list,
			Maia::Renderer::D3D12::Upload_buffer_view upload_buffer_view,
			std::pair<float, float> display_size
		) noexcept;

		void upload_user_interface_data(
			Frame_index const frame_index
		) noexcept;

		void execute_user_interface_pass(
			ID3D12GraphicsCommandList& command_list,
			Maia::Renderer::D3D12::Constant_buffer_view const pass_buffer_view
		) noexcept;


	private:

		ImGuiContext* m_context;
		Maia::Renderer::D3D12::Non_rt_ds_image_heap m_image_heap;
		Maia::Renderer::D3D12::Sampled_image_2d m_font_texture;
		Maia::Renderer::D3D12::Cbv_srv_uav_descriptor_heap m_descriptor_heap;
		Maia::Renderer::D3D12::Root_signature m_root_signature;
		Maia::Renderer::D3D12::Cbv_srv_uav_descriptor_table m_texture_descriptor_table;
		Maia::Renderer::D3D12::Upload_buffer_heap m_geometry_buffer_heap;
		Maia::Renderer::D3D12::Dynamic_geometry_buffer m_geometry_buffer;

	};
}

#endif
