#ifndef MAIA_RENDERER_BUFFERVIEW_H_INCLUDED
#define MAIA_RENDERER_BUFFERVIEW_H_INCLUDED

#include <d3d12.h>

namespace Maia::Renderer::D3D12
{
	class Buffer_view
	{
	public:

		Buffer_view(ID3D12Resource& resource, UINT64 offset, UINT64 size) noexcept;


		Buffer_view sub_view(UINT64 offset, UINT64 size) const noexcept;


		ID3D12Resource& resource() const noexcept;
		UINT64 offset() const noexcept;
		UINT64 size() const noexcept;


	private:

		ID3D12Resource& m_resource;
		UINT64 m_offset;
		UINT64 m_size;

	};
}

#endif