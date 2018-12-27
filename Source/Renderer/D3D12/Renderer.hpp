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
	class Renderer
	{
	public:

		Renderer(IDXGIFactory6& factory, Render_resources& render_resources, Eigen::Vector2i viewport_and_scissor_dimensions, std::uint8_t pipeline_length);

		void resize_viewport_and_scissor_rects(Eigen::Vector2i dimensions);

		void wait();

		void render(ID3D12Resource& render_target, D3D12_CPU_DESCRIPTOR_HANDLE render_target_descriptor_handle, Scene_resources const& scene_resources);

	private:

		std::uint8_t m_pipeline_length;
		Render_resources& m_render_resources;
		UINT64 m_fence_value;
		winrt::com_ptr<ID3D12Fence> m_fence;
		winrt::handle m_fence_event;
		D3D12_VIEWPORT m_viewport;
		D3D12_RECT m_scissor_rect;
		std::size_t m_submitted_frames;
		winrt::com_ptr<ID3D12RootSignature> m_root_signature;
		Maia::Renderer::D3D12::Shader m_color_vertex_shader;
		Maia::Renderer::D3D12::Shader m_color_pixel_shader;
		winrt::com_ptr<ID3D12PipelineState> m_color_pass_pipeline_state;

	};
}

#endif
