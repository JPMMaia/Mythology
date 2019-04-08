#include <Maia/Renderer/D3D12/Utilities/D3D12_utilities.hpp>

#include "Render_data.hpp"

using namespace Maia::Renderer::D3D12;

namespace Maia::Mythology::D3D12
{
	Frames_resources::Frames_resources(ID3D12Device& device, UINT pipeline_length) :
		rtv_descriptor_heap{ create_descriptor_heap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, pipeline_length, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0) }
	{
	}
}
