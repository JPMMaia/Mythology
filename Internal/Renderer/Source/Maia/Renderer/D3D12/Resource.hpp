#ifndef MAIA_RENDERER_RESOURCE_H_INCLUDED
#define MAIA_RENDERER_RESOURCE_H_INCLUDED

#include <optional>

#include <d3d12.h>

#include <winrt/base.h>

#include <Maia/Renderer/D3D12/Heap_view.hpp>

namespace Maia::Renderer::D3D12
{
	struct Buffer_offset
	{
		UINT64 value;
	};

	struct Buffer_size
	{
		UINT64 value;
	};

	struct Image_width
	{
		UINT64 value;
	};

	struct Image_height
	{
		UINT value;
	};

	struct Image_depth
	{
	};

	struct First_mip_level
	{
		UINT16 value;
	};

	struct Mip_levels
	{
		UINT16 value;
	};

	struct First_array_slice
	{
		UINT16 value;
	};

	struct Array_slices
	{
		UINT16 value;
	};


	struct Resource
	{
		winrt::com_ptr<ID3D12Resource> value;
	};


	struct Buffer : public Resource
	{
	};

	struct Default_buffer : public Buffer
	{
		Default_buffer(
			ID3D12Device& device,
			Default_buffer_heap_view heap_view,
			D3D12_RESOURCE_STATES initial_state = D3D12_RESOURCE_STATE_COPY_DEST
		) noexcept;
	};

	struct Readback_buffer : public Buffer
	{
		Readback_buffer(
			ID3D12Device& device,
			Readback_buffer_heap_view heap_view
		) noexcept;
	};

	struct Upload_buffer : public Buffer
	{
		Upload_buffer(
			ID3D12Device& device,
			Upload_buffer_heap_view heap_view
		) noexcept;
	};

	struct Dynamic_geometry_buffer : public Buffer
	{
		Dynamic_geometry_buffer(
			ID3D12Device& device,
			Upload_buffer_heap_view heap_view
		) noexcept;
	};

	struct Non_dynamic_geometry_buffer : public Buffer
	{
		Non_dynamic_geometry_buffer(
			ID3D12Device& device,
			Upload_buffer_heap_view heap_view
		) noexcept;
	};



	struct Image : public Resource {};
	struct Image_2d : public Image {};


	struct Sampled_image : public Image {};

	struct Sampled_image_2d : public Image_2d
	{
		Sampled_image_2d(
			ID3D12Device& device,
			Non_rt_ds_image_heap_view heap_view,
			DXGI_FORMAT format,
			std::tuple<Image_width, Image_height> dimensions,
			Array_slices array_slices,
			Mip_levels mip_levels,
			D3D12_RESOURCE_STATES initial_state = D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE
		) noexcept;
	};


	struct Unordered_access_image : public Image {};

	struct Unordered_access_image_2d : public Image_2d
	{
		Unordered_access_image_2d(
			ID3D12Device& device,
			Non_rt_ds_image_heap_view heap_view,
			DXGI_FORMAT format,
			std::tuple<Image_width, Image_height> dimensions,
			Array_slices array_slices,
			Mip_levels mip_levels,
			D3D12_RESOURCE_STATES initial_state = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
		) noexcept;
	};


	struct Render_target_image : public Image {};

	struct Render_target_image_2d : public Image_2d
	{
		Render_target_image_2d(
			ID3D12Device& device,
			Rt_ds_image_heap_view heap_view,
			DXGI_FORMAT format,
			std::tuple<Image_width, Image_height> dimensions,
			Array_slices array_slices,
			Mip_levels mip_levels,
			D3D12_CLEAR_VALUE optimized_clear_value,
			D3D12_RESOURCE_STATES initial_state = D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
		) noexcept;
	};


	struct Depth_stencil_image : public Image {};

	struct Depth_stencil_image_2d : public Image_2d
	{
		Depth_stencil_image_2d(
			ID3D12Device& device,
			Rt_ds_image_heap_view heap_view,
			DXGI_FORMAT format,
			std::tuple<Image_width, Image_height> dimensions,
			Array_slices array_slices,
			Mip_levels mip_levels,
			D3D12_CLEAR_VALUE optimized_clear_value,
			D3D12_RESOURCE_STATES initial_state = D3D12_RESOURCE_STATE_DEPTH_WRITE,
			D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
		) noexcept;
	};
}

#endif
