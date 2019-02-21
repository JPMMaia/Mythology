#ifndef MAIA_MYTHOLOGY_D3D12_RENDERER_H_INCLUDED
#define MAIA_MYTHOLOGY_D3D12_RENDERER_H_INCLUDED

#include <cstddef>

#include <Eigen/Eigen>
#include <winrt/base.h>

#include <dxgi1_6.h>
#include <d3d12.h>

#include <Maia/Renderer/D3D12/Utilities/Shader.hpp>

namespace Maia::Mythology::D3D12
{
	// TODO rename to Render_frame_system ?
	class Renderer
	{
	public:

		Renderer(
			ID3D12Device& device,
			Eigen::Vector2i viewport_and_scissor_dimensions,
			std::uint8_t pipeline_length
		);

		void resize_viewport_and_scissor_rects(Eigen::Vector2i dimensions);

		ID3D12CommandList& render(
			std::uint8_t current_frame_index,
			ID3D12Resource& render_target,
			D3D12_CPU_DESCRIPTOR_HANDLE render_target_descriptor_handle,
			gsl::span<Mesh_view const> mesh_views,
			gsl::span<D3D12_VERTEX_BUFFER_VIEW const> instance_buffer_views,
			D3D12_GPU_VIRTUAL_ADDRESS pass_data_buffer_address
		);

	private:

		std::vector<winrt::com_ptr<ID3D12CommandAllocator>> m_command_allocators;
		winrt::com_ptr<ID3D12GraphicsCommandList> m_command_list;

		D3D12_VIEWPORT m_viewport;
		D3D12_RECT m_scissor_rect;
		winrt::com_ptr<ID3D12RootSignature> m_root_signature;
		Maia::Renderer::D3D12::Shader m_color_vertex_shader;
		Maia::Renderer::D3D12::Shader m_color_pixel_shader;
		winrt::com_ptr<ID3D12PipelineState> m_color_pass_pipeline_state;

	};
}

#endif
