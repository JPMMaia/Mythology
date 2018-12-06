#ifndef MAIA_MYTHOLOGY_D3D12_RENDERER_H_INCLUDED
#define MAIA_MYTHOLOGY_D3D12_RENDERER_H_INCLUDED

#include <cstddef>

#include <winrt/base.h>

#include <dxgi1_6.h>
#include <d3d12.h>

namespace Mythology::D3D12
{
	class Renderer
	{
	public:

		explicit Renderer(IUnknown& window);

		void render();
		void present();

	private:

		std::uint8_t const m_pipeline_length;
		winrt::com_ptr<IDXGIFactory6> m_factory;
		winrt::com_ptr<IDXGIAdapter> m_adapter;
		winrt::com_ptr<ID3D12Device> m_device;
		winrt::com_ptr<ID3D12CommandQueue> m_direct_command_queue;
		std::vector<winrt::com_ptr<ID3D12CommandAllocator>> m_command_allocators;
		winrt::com_ptr<ID3D12GraphicsCommandList> m_command_list;
		winrt::com_ptr<ID3D12DescriptorHeap> m_rtv_descriptor_heap;
		UINT64 m_fence_value;
		winrt::com_ptr<ID3D12Fence> m_fence;
		winrt::handle m_fence_event;
		winrt::com_ptr<IDXGISwapChain4> m_swap_chain;
		std::size_t m_submitted_frames;

	};
}

#endif
