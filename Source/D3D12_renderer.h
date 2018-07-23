#ifndef MYTHOLOGY_D3D12_RENDERER_H_INCLUDED
#define MYTHOLOGY_D3D12_RENDERER_H_INCLUDED

#include <winrt/base.h>

#include <d3d12.h>

namespace Mythology
{
	class D3D12_renderer
	{
	public:

		D3D12_renderer();

	private:

		winrt::com_ptr<IDXGIFactory6> m_factory;
		winrt::com_ptr<IDXGIAdapter> m_adapter;
		winrt::com_ptr<ID3D12Device> m_device;
		winrt::com_ptr<ID3D12CommandQueue> m_render_command_queue;
		winrt::com_ptr<ID3D12CommandAllocator> m_command_allocator;
		winrt::com_ptr<ID3D12CommandList> m_command_list;
		winrt::com_ptr<ID3D12DescriptorHeap> m_descriptor_heap;
		winrt::com_ptr<ID3D12Fence> m_fence;
		winrt::handle m_fence_event;
		UINT64 m_fence_value;

	};
}

#endif
