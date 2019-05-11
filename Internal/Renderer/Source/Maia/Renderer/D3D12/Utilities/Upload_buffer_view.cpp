#include "Upload_buffer_view.hpp"

#include <cassert>

namespace Maia::Renderer::D3D12
{
	Upload_buffer_view::Upload_buffer_view(ID3D12Resource& resource, UINT64 const offset, UINT64 const size) noexcept :
		m_view{ resource, offset, size }
	{
	}


	Upload_buffer_view Upload_buffer_view::sub_view(UINT64 const offset, UINT64 const size) const noexcept
	{
		assert(this->offset() + offset + size <= this->size());

		return { this->resource(), this->offset() + offset, size };
	}


	ID3D12Resource& Upload_buffer_view::resource() const noexcept
	{
		return m_view.resource();
	}
	UINT64 Upload_buffer_view::offset() const noexcept
	{
		return m_view.offset();
	}
	UINT64 Upload_buffer_view::size() const noexcept
	{
		return m_view.size();
	}
}
