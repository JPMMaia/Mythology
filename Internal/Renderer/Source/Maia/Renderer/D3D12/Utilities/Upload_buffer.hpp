#ifndef MAIA_RENDERER_UPLOADBUFFER_H_INCLUDED
#define MAIA_RENDERER_UPLOADBUFFER_H_INCLUDED

#include <d3d12.h>

#include <winrt/base.h>

namespace Maia::Renderer::D3D12
{
	class Upload_buffer_view;

	class Upload_buffer
	{
	public:

		Upload_buffer(ID3D12Device& device, UINT64 size = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT) noexcept;


		Upload_buffer_view view(UINT64 offset, UINT64 size) const noexcept;


	private:

		winrt::com_ptr<ID3D12Heap> m_heap;
		winrt::com_ptr<ID3D12Resource> m_resource;
		UINT64 m_size;
	};
}

#endif