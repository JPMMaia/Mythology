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

		winrt::com_ptr<ID3D12Device> m_device;
		winrt::com_ptr<ID3D12CommandQueue> m_renderCommandQueue;
		winrt::com_ptr<ID3D12CommandAllocator> m_commandAllocator;
		winrt::com_ptr<ID3D12CommandList> m_commandList;
		winrt::com_ptr<ID3D12DescriptorHeap> m_descriptorHeap;
		winrt::com_ptr<ID3D12Fence> m_fence;
		winrt::handle m_fenceEvent;
		UINT64 m_fenceValue;

	};
}

#endif
