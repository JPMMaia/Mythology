#ifndef MAIA_RENDERER_DESCRIPTORHEAP_H_INCLUDED
#define MAIA_RENDERER_DESCRIPTORHEAP_H_INCLUDED

#include <d3d12.h>

#include <winrt/base.h>

namespace Maia::Renderer::D3D12
{
	struct Descriptor_heap_offset
	{
		UINT value;
	};

	struct Descriptor_heap_size
	{
		UINT value;
	};


	struct Descriptor_heap
	{
		winrt::com_ptr<ID3D12DescriptorHeap> value;
	};

	struct Cbv_srv_uav_descriptor_heap : public Descriptor_heap
	{
		Cbv_srv_uav_descriptor_heap(
			ID3D12Device& device,
			Descriptor_heap_size size,
			UINT node_mask = {}
		) noexcept;
	};

	struct Rtv_descriptor_heap : public Descriptor_heap
	{
		Rtv_descriptor_heap(
			ID3D12Device& device,
			Descriptor_heap_size size,
			UINT node_mask = {}
		) noexcept;
	};

	struct Dsv_descriptor_heap : public Descriptor_heap
	{
		Dsv_descriptor_heap(
			ID3D12Device& device,
			Descriptor_heap_size const size,
			UINT node_mask = {}
		) noexcept;
	};

	struct Non_shader_visible_uav_descriptor_heap : public Descriptor_heap
	{
		Non_shader_visible_uav_descriptor_heap(
			ID3D12Device& device,
			Descriptor_heap_size const size,
			UINT node_mask = {}
		) noexcept;
	};

	struct Sampler_descriptor_heap : public Descriptor_heap
	{
		Sampler_descriptor_heap(
			ID3D12Device& device,
			Descriptor_heap_size const size,
			UINT node_mask = {}
		) noexcept;
	};
}

#endif