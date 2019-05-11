#include "Upload_buffer.hpp"

#include <Maia/Renderer/D3D12/Utilities/D3D12_utilities.hpp>
#include <Maia/Renderer/D3D12/Utilities/Upload_buffer_view.hpp>

namespace Maia::Renderer::D3D12
{
	Upload_buffer::Upload_buffer(ID3D12Device& device, UINT64 const size) noexcept :
		m_heap{ create_upload_heap(device, size) },
		m_resource{ create_buffer(device, *m_heap, 0, size, D3D12_RESOURCE_STATE_GENERIC_READ) },
		m_size{ size }
	{
	}


	Upload_buffer_view Upload_buffer::view(UINT64 const offset, UINT64 const size) const noexcept
	{
		return { *m_resource, offset, size };
	}
}
