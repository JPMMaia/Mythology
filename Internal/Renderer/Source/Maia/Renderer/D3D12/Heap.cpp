#include "Heap.hpp"

#include <d3dx12.h>

#include <Maia/Renderer/D3D12/Utilities/Check_hresult.hpp>

namespace Maia::Renderer::D3D12
{
	namespace
	{
		struct Heap_alignment
		{
			UINT64 value{ D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT };
		};

		[[nodiscard]] winrt::com_ptr<ID3D12Heap> create_heap(
			ID3D12Device& device, 
			D3D12_HEAP_TYPE const heap_type,
			Heap_size const size,
			Heap_alignment const alignment,
			D3D12_HEAP_FLAGS const flags
		) noexcept
		{
			assert(size.value % alignment.value == 0);

			CD3DX12_HEAP_PROPERTIES const properties{ heap_type };
			CD3DX12_HEAP_DESC const description{ size.value, properties, alignment.value, flags };

			winrt::com_ptr<ID3D12Heap> heap;
			check_hresult(
				device.CreateHeap(&description, __uuidof(heap), heap.put_void()));

			return heap;
		}
	}


	Readback_buffer_heap::Readback_buffer_heap(
		ID3D12Device& device,
		Heap_size const size
	) noexcept :
		Heap{ create_heap(device, D3D12_HEAP_TYPE_READBACK, size, { D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT }, D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS) }
	{
	}


	Upload_buffer_heap::Upload_buffer_heap(
		ID3D12Device& device,
		Heap_size const size
	) noexcept :
		Heap{ create_heap(device, D3D12_HEAP_TYPE_UPLOAD, size, { D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT }, D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS) }
	{
	}


	Non_rt_ds_image_heap::Non_rt_ds_image_heap(
		ID3D12Device& device,
		Heap_size const size
	) noexcept :
		Heap{ create_heap(device, D3D12_HEAP_TYPE_DEFAULT, size, { D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT }, D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES) }
	{
	}


	Rt_ds_image_heap::Rt_ds_image_heap(
		ID3D12Device& device,
		Heap_size const size
	) noexcept :
		Heap{ create_heap(device, D3D12_HEAP_TYPE_DEFAULT, size, { D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT }, D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES) }
	{
	}
}
