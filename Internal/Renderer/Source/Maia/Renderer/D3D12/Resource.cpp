#include "Resource.hpp"

#include <d3dx12.h>

#include <Maia/Renderer/D3D12/Utilities/Check_hresult.hpp>

namespace Maia::Renderer::D3D12
{
	namespace
	{
		template <class Heap_view>
		[[nodiscard]] winrt::com_ptr<ID3D12Resource> create_buffer(
			ID3D12Device& device,
			Heap_view const heap_view,
			D3D12_RESOURCE_STATES const initial_state
		)
		{
			assert(heap_view.offset.value % D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT == 0);
			assert(heap_view.size.value % D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT == 0);

			CD3DX12_RESOURCE_DESC const description = CD3DX12_RESOURCE_DESC::Buffer(heap_view.size.value);

			winrt::com_ptr<ID3D12Resource> buffer;
			check_hresult(
				device.CreatePlacedResource(
					heap_view.heap.value.get(),
					heap_view.offset.value,
					&description,
					initial_state,
					nullptr,
					__uuidof(buffer),
					buffer.put_void()
				)
			);

			return buffer;
		}
	}

	Default_buffer::Default_buffer(
		ID3D12Device& device,
		Default_buffer_heap_view const heap_view,
		D3D12_RESOURCE_STATES const initial_state
	) noexcept :
		Buffer{ create_buffer(device, heap_view, D3D12_RESOURCE_STATE_COPY_DEST) }
	{
	}

	Readback_buffer::Readback_buffer(
		ID3D12Device& device,
		Readback_buffer_heap_view const heap_view
	) noexcept :
		Buffer{ create_buffer(device, heap_view, D3D12_RESOURCE_STATE_COPY_DEST) }
	{
	}

	Upload_buffer::Upload_buffer(
		ID3D12Device& device,
		Upload_buffer_heap_view const heap_view
	) noexcept :
		Buffer{ create_buffer(device, heap_view, D3D12_RESOURCE_STATE_GENERIC_READ) }
	{
	}


	namespace
	{
		template <class Heap_view>
		[[nodiscard]] winrt::com_ptr<ID3D12Resource> create_image_2d(
			ID3D12Device& device,
			Heap_view const heap_view,
			DXGI_FORMAT const format,
			std::tuple<Image_width, Image_height> const dimensions,
			Array_slices const array_slices,
			Mip_levels const mip_levels,
			D3D12_RESOURCE_STATES const initial_state,
			D3D12_RESOURCE_FLAGS const flags,
			std::optional<D3D12_CLEAR_VALUE> optimized_clear_value = {}
		) noexcept
		{
			assert(heap_view.offset.value % D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT == 0);
			assert(heap_view.size.value % D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT == 0);

			CD3DX12_RESOURCE_DESC const description = CD3DX12_RESOURCE_DESC::Tex2D(
				format,
				std::get<0>(dimensions).value,
				std::get<1>(dimensions).value,
				array_slices.value,
				mip_levels.value,
				1, 0,
				flags
			);

			winrt::com_ptr<ID3D12Resource> image;
			check_hresult(
				device.CreatePlacedResource(
					heap_view.heap.value.get(),
					heap_view.offset.value,
					&description,
					initial_state,
					optimized_clear_value ? nullptr : &optimized_clear_value.value(),
					__uuidof(image),
					image.put_void()
				)
			);

			return image;
		}
	}

	Sampled_image_2d::Sampled_image_2d(
		ID3D12Device& device,
		Non_rt_ds_image_heap_view const heap_view,
		DXGI_FORMAT const format,
		std::tuple<Image_width, Image_height> const dimensions,
		Array_slices const array_slices,
		Mip_levels const mip_levels,
		D3D12_RESOURCE_STATES const initial_state,
		D3D12_RESOURCE_FLAGS const flags
	) noexcept :
		Image_2d{ create_image_2d(device, heap_view, format, dimensions, array_slices, mip_levels, initial_state, flags) }
	{
	}

	Unordered_access_image_2d::Unordered_access_image_2d(
		ID3D12Device& device,
		Non_rt_ds_image_heap_view const heap_view,
		DXGI_FORMAT const format,
		std::tuple<Image_width, Image_height> const dimensions,
		Array_slices const array_slices,
		Mip_levels const mip_levels,
		D3D12_RESOURCE_STATES const initial_state,
		D3D12_RESOURCE_FLAGS const flags
	) noexcept :
		Image_2d{ create_image_2d(device, heap_view, format, dimensions, array_slices, mip_levels, initial_state, flags) }
	{
	}

	Render_target_image_2d::Render_target_image_2d(
		ID3D12Device& device,
		Rt_ds_image_heap_view const heap_view,
		DXGI_FORMAT const format,
		std::tuple<Image_width, Image_height> const dimensions,
		Array_slices const array_slices,
		Mip_levels const mip_levels,
		D3D12_CLEAR_VALUE const optimized_clear_value,
		D3D12_RESOURCE_STATES const initial_state,
		D3D12_RESOURCE_FLAGS const flags
	) noexcept :
		Image_2d{ create_image_2d(device, heap_view, format, dimensions, array_slices, mip_levels, initial_state, flags, optimized_clear_value) }
	{
	}

	Depth_stencil_image_2d::Depth_stencil_image_2d(
		ID3D12Device& device,
		Rt_ds_image_heap_view const heap_view,
		DXGI_FORMAT const format,
		std::tuple<Image_width, Image_height> const dimensions,
		Array_slices const array_slices,
		Mip_levels const mip_levels,
		D3D12_CLEAR_VALUE const optimized_clear_value,
		D3D12_RESOURCE_STATES const initial_state,
		D3D12_RESOURCE_FLAGS const flags
	) noexcept :
		Image_2d{ create_image_2d(device, heap_view, format, dimensions, array_slices, mip_levels, initial_state, flags, optimized_clear_value) }
	{
	}
}
