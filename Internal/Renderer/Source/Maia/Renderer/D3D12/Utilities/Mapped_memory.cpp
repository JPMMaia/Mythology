#include <winrt/base.h>

#include "Check_hresult.hpp"
#include <Maia/Renderer/D3D12/Utilities/Mapped_memory.hpp>

namespace Maia::Renderer::D3D12
{
	namespace
	{
		void* map(ID3D12Resource& resource, UINT subresource, D3D12_RANGE read_range)
		{
			void* mapped_memory;
			
			check_hresult(
				resource.Map(subresource, &read_range, &mapped_memory));
			
			return mapped_memory;
		}

		void unmap(ID3D12Resource& resource, UINT subresource, D3D12_RANGE written_range)
		{
			resource.Unmap(subresource, &written_range);
		}
	}

	Mapped_memory::Mapped_memory(ID3D12Resource& resource, UINT subresource, D3D12_RANGE read_range) :
		m_resource{ resource },
		m_subresource{ subresource },
		m_mapped_memory{ map(resource, subresource, read_range) },
		m_written_range{ static_cast<SIZE_T>(-1), 0 }
	{
	}
	Mapped_memory::~Mapped_memory()
	{
		unmap(m_resource, m_subresource, m_written_range);
	}
}
