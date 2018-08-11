#ifndef MYTHOLOGY_D3D12_RENDERER_H_INCLUDED
#define MYTHOLOGY_D3D12_RENDERER_H_INCLUDED

#include <cstddef>

#include <winrt/base.h>

#include <dxgi1_6.h>
#include <d3d12_1.h>

namespace Mythology
{
	class D3D12_renderer
	{
	public:

		D3D12_renderer();

		void window(IUnknown& window);

	private:

		std::size_t const m_pipeline_length;
		winrt::com_ptr<IDXGIFactory6> m_factory;
		winrt::com_ptr<IDXGIAdapter> m_adapter;
		winrt::com_ptr<ID3D12Device> m_device;
		winrt::com_ptr<ID3D12CommandQueue> m_direct_command_queue;
		winrt::com_ptr<ID3D12CommandAllocator> m_command_allocator;
		winrt::com_ptr<ID3D12CommandList> m_command_list;
		winrt::com_ptr<ID3D12DescriptorHeap> m_rtv_descriptor_heap;
		UINT64 m_fence_value;
		winrt::com_ptr<ID3D12Fence> m_fence;
		winrt::handle m_fence_event;
		winrt::com_ptr<IDXGISwapChain> m_swap_chain;

	};
}

#endif
