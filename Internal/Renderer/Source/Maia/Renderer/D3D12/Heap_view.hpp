#ifndef MAIA_RENDERER_HEAPVIEW_H_INCLUDED
#define MAIA_RENDERER_HEAPVIEW_H_INCLUDED

#include <d3d12.h>

#include <Maia/Renderer/D3D12/Heap.hpp>

namespace Maia::Renderer::D3D12
{
	template <class Heap>
	struct Heap_view
	{
		Heap& heap;
		Heap_offset offset;
		Heap_size size;
	};

	struct Default_buffer_heap_view : public Heap_view<Readback_buffer_heap> {};
	struct Readback_buffer_heap_view : public Heap_view<Readback_buffer_heap> {};
	struct Upload_buffer_heap_view : public Heap_view<Upload_buffer_heap> {};

	struct Non_rt_ds_image_heap_view : public Heap_view<Non_rt_ds_image_heap> {};
	struct Rt_ds_image_heap_view : public Heap_view<Rt_ds_image_heap> {};
}

#endif