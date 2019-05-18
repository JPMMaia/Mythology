#ifndef MAIA_RENDERER_RESOURCEVIEW_H_INCLUDED
#define MAIA_RENDERER_RESOURCEVIEW_H_INCLUDED

#include <d3d12.h>

#include <winrt/base.h>

#include <Maia/Renderer/D3D12/Resource.hpp>

namespace Maia::Renderer::D3D12
{
	template <class Buffer>
	struct Buffer_view
	{
		Buffer& buffer;
		Buffer_offset offset;
		Buffer_size size;
	};

	struct Constant_buffer_view : public Buffer_view<Default_buffer> {};
	struct Unordered_access_buffer_view : public Buffer_view<Default_buffer> {};
	struct Readback_buffer_view : public Buffer_view<Readback_buffer> {};
	struct Upload_buffer_view : public Buffer_view<Upload_buffer> {};


	template <class Image>
	struct Image_view
	{
		Image& image;
		First_mip_level first_mip_level;
		Mip_levels mip_levels;
	};

	struct Sampled_image_view : public Image_view<Image> {};
	struct Sampled_image_2d_view : public Image_view<Image_2d> {};
	
	struct Unordered_access_image_view : public Image_view<Unordered_access_image> {};
	struct Unordered_access_image_2d_view : public Image_view<Unordered_access_image_2d> {};
	
	struct Render_target_image_view : public Image_view<Render_target_image> {};
	struct Render_target_image_2d_view : public Image_view<Render_target_image_2d> {};
	
	struct Depth_stencil_image_view : public Image_view<Depth_stencil_image> {};
	struct Depth_stencil_image_2d_view : public Image_view<Depth_stencil_image_2d> {};


	template <class Image>
	struct Image_array_view
	{
		Image& image;
		First_mip_level first_mip_level;
		Mip_levels mip_levels;
		First_array_slice first_array_slice;
		Array_slices array_slices;
	};

	struct Sampled_image_array_view : public Image_array_view<Image> {};
	struct Sampled_image_2d_array_view : public Image_array_view<Image_2d> {};
	
	struct Unordered_access_image_array_view : public Image_array_view<Unordered_access_image> {};
	struct Unordered_access_image_2d_array_view : public Image_array_view<Unordered_access_image_2d> {};
	
	struct Render_target_image_array_view : public Image_array_view<Render_target_image> {};
	struct Render_target_image_2d_array_view : public Image_array_view<Render_target_image_2d> {};
	
	struct Depth_stencil_image_array_view : public Image_array_view<Depth_stencil_image> {};
	struct Depth_stencil_image_2d_array_view : public Image_array_view<Depth_stencil_image_2d> {};
}

#endif
