#include "Descriptor_heap.hpp"

#include <Maia/Renderer/D3D12/Utilities/Check_hresult.hpp>

namespace Maia::Renderer::D3D12
{
	namespace
	{
		[[nodiscard]] winrt::com_ptr<ID3D12DescriptorHeap> create_descriptor_heap(
			ID3D12Device& device,
			D3D12_DESCRIPTOR_HEAP_TYPE const type,
			Descriptor_heap_size const size,
			D3D12_DESCRIPTOR_HEAP_FLAGS const flags,
			UINT const node_mask
		) noexcept
		{
			D3D12_DESCRIPTOR_HEAP_DESC description;
			description.Type = type;
			description.NumDescriptors = size.value;
			description.Flags = flags;
			description.NodeMask = node_mask;

			winrt::com_ptr<ID3D12DescriptorHeap> descriptor_heap;
			check_hresult(
				device.CreateDescriptorHeap(&description, __uuidof(descriptor_heap), descriptor_heap.put_void()));

			return descriptor_heap;
		}
	}

	Cbv_srv_uav_descriptor_heap::Cbv_srv_uav_descriptor_heap(
		ID3D12Device& device,
		Descriptor_heap_size const size,
		UINT const node_mask
	) noexcept :
		Descriptor_heap{ create_descriptor_heap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, size, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, node_mask) }
	{
	}

	Rtv_descriptor_heap::Rtv_descriptor_heap(
		ID3D12Device& device,
		Descriptor_heap_size const size,
		UINT const node_mask
	) noexcept :
		Descriptor_heap{ create_descriptor_heap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, size, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, node_mask) }
	{
	}

	Dsv_descriptor_heap::Dsv_descriptor_heap(
		ID3D12Device& device,
		Descriptor_heap_size const size,
		UINT const node_mask
	) noexcept :
		Descriptor_heap{ create_descriptor_heap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, size, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, node_mask) }
	{
	}

	Non_shader_visible_uav_descriptor_heap::Non_shader_visible_uav_descriptor_heap(
		ID3D12Device& device,
		Descriptor_heap_size const size,
		UINT const node_mask
	) noexcept :
		Descriptor_heap{ create_descriptor_heap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, size, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, node_mask) }
	{
	}

	Sampler_descriptor_heap::Sampler_descriptor_heap(
		ID3D12Device& device,
		Descriptor_heap_size const size,
		UINT const node_mask
	) noexcept :
		Descriptor_heap{ create_descriptor_heap(device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, size, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, node_mask) }
	{
	}
}
