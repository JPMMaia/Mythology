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
	struct Triangle
	{
		winrt::com_ptr<ID3D12Resource> vertex_buffer;
		winrt::com_ptr<ID3D12Resource> index_buffer;
	};

	class Renderer
	{
	public:

		Renderer(IDXGIFactory6& factory, Render_resources& render_resources, IUnknown& window, Eigen::Vector2i window_dimensions);

		void resize_window(Eigen::Vector2i window_dimensions);

		void render(Scene_resources const& scene_resources);
		void present();

	private:

		static constexpr std::uint8_t m_pipeline_length{ 3 };
		static constexpr std::uint8_t m_swap_chain_buffer_count{ 3 };
		Render_resources& m_render_resources;
		winrt::com_ptr<ID3D12DescriptorHeap> m_rtv_descriptor_heap;
		UINT64 m_fence_value;
		winrt::com_ptr<ID3D12Fence> m_fence;
		winrt::handle m_fence_event;
		winrt::com_ptr<IDXGISwapChain4> m_swap_chain;
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
