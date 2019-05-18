#ifndef MAIA_RENDERER_HEAP_H_INCLUDED
#define MAIA_RENDERER_HEAP_H_INCLUDED

#include <d3d12.h>

#include <winrt/base.h>

namespace Maia::Renderer::D3D12
{
	struct Heap_offset
	{
		UINT64 value{ 0 };
	};

	struct Heap_size
	{
		UINT64 value{ D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT };
	};


	struct Heap
	{
		winrt::com_ptr<ID3D12Heap> value;
	};

	struct Readback_buffer_heap : public Heap
	{
		Readback_buffer_heap(
			ID3D12Device& device,
			Heap_size size
		) noexcept;
	};

	struct Upload_buffer_heap : public Heap
	{
		Upload_buffer_heap(
			ID3D12Device& device,
			Heap_size size
		) noexcept;
	};

	struct Non_rt_ds_image_heap : public Heap
	{
		Non_rt_ds_image_heap(
			ID3D12Device& device,
			Heap_size size
		) noexcept;
	};

	struct Rt_ds_image_heap : public Heap
	{
		Rt_ds_image_heap(
			ID3D12Device& device,
			Heap_size size
		) noexcept;
	};
}

#endif
