#ifndef MAIA_RENDERER_DESCRIPTORHEAPVIEW_H_INCLUDED
#define MAIA_RENDERER_DESCRIPTORHEAPVIEW_H_INCLUDED

#include <Maia/Renderer/D3D12/Descriptor_heap.hpp>

namespace Maia::Renderer::D3D12
{
	template <class Descriptor_heap>
	struct Descriptor_heap_view
	{
		Descriptor_heap& descriptor_heap;
		Descriptor_heap_offset offset;
		Descriptor_heap_size size;
	};

	struct Cbv_srv_uav_descriptor_heap_view : public Descriptor_heap_view<Cbv_srv_uav_descriptor_heap> {};
	struct Rtv_descriptor_heap_view : public Descriptor_heap_view<Rtv_descriptor_heap> {};
	struct Dsv_descriptor_heap_view : public Descriptor_heap_view<Dsv_descriptor_heap> {};
	struct Non_shader_visible_uav_descriptor_heap_view : public Descriptor_heap_view<Non_shader_visible_uav_descriptor_heap> {};
	struct Sampler_descriptor_heap_view : public Descriptor_heap_view<Sampler_descriptor_heap> {};
}

#endif