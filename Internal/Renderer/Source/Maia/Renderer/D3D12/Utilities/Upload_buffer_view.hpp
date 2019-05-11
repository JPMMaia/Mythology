#ifndef MAIA_RENDERER_UPLOADBUFFERVIEW_H_INCLUDED
#define MAIA_RENDERER_UPLOADBUFFERVIEW_H_INCLUDED

#include <d3d12.h>

#include <Maia/Renderer/D3D12/Utilities/Buffer_view.hpp>

namespace Maia::Renderer::D3D12
{
	class Upload_buffer_view
	{
	public:

		Upload_buffer_view(ID3D12Resource& resource, UINT64 offset, UINT64 size) noexcept;


		Upload_buffer_view sub_view(UINT64 offset, UINT64 size) const noexcept;


		ID3D12Resource& resource() const noexcept;
		UINT64 offset() const noexcept;
		UINT64 size() const noexcept;

	private:

		Buffer_view m_view;

	};
}

#endif