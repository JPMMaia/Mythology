#ifndef MAIA_GAMEENGINE_RENDERDATA_H_INCLUDED
#define MAIA_GAMEENGINE_RENDERDATA_H_INCLUDED

#include <vector>

#include <winrt/base.h>

#include <d3d12.h>

namespace Maia::Mythology::D3D12
{
	struct Geometry_and_instances_buffer
	{
		winrt::com_ptr<ID3D12Resource> value;
	};

	struct Instance_count
	{
		UINT value;
	};

	struct Render_primitive
	{
		std::vector<D3D12_VERTEX_BUFFER_VIEW> vertex_buffer_views;
		D3D12_INDEX_BUFFER_VIEW index_buffer_view;
		D3D12_GPU_VIRTUAL_ADDRESS instances_location;
		UINT index_count;
		UINT instance_count;
	};

	struct Render_resources
	{
		std::vector<Render_primitive> primitives;
	};
}

#endif
