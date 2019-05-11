#include "Buffer_view.hpp"

#include <cassert>

namespace Maia::Renderer::D3D12
{
	Buffer_view::Buffer_view(ID3D12Resource& resource, UINT64 const offset, UINT64 const size) noexcept :
		m_resource{ resource },
		m_offset{ offset },
		m_size{ size }
	{
	}


	Buffer_view Buffer_view::sub_view(UINT64 const offset, UINT64 const size) const noexcept
	{
		assert(this->offset() + offset + size < this->size());

		return { this->resource(), this->offset() + offset, size };
	}


	ID3D12Resource& Buffer_view::resource() const noexcept
	{
		return m_resource;
	}
	UINT64 Buffer_view::offset() const noexcept
	{
		return m_offset;
	}
	UINT64 Buffer_view::size() const noexcept
	{
		return m_size;
	}
}
