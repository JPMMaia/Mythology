#ifndef MAIA_RENDERER_MAPPEDMEMORY_H_INCLUDED
#define MAIA_RENDERER_MAPPEDMEMORY_H_INCLUDED

#include <d3d12.h>

#include <cstddef>
#include <cstring>

namespace Maia::Renderer::D3D12
{
	class Mapped_memory
	{
	public:

		Mapped_memory(ID3D12Resource& resource, UINT subresource, D3D12_RANGE read_range);
		~Mapped_memory();

		void write(void const* data, std::size_t size, SIZE_T begin = 0)
		{
			void* mapped_memory = reinterpret_cast<std::byte*>(m_mapped_memory) + begin;
			std::memcpy(mapped_memory, data, size);

			if (m_written_range.Begin > begin)
				m_written_range.Begin = begin;

			const auto end = begin + size;
			if (m_written_range.End < end)
				m_written_range.End = end;
		}

		template <class T>
		void write(const T& data, SIZE_T begin = 0)
		{
			write(&data, sizeof(data), begin);
		}

	private:

		ID3D12Resource& m_resource;
		UINT m_subresource;
		void* m_mapped_memory;
		D3D12_RANGE m_written_range;

	};
}

#endif