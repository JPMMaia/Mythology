#include <Maia/Renderer/D3D12/Utilities/D3D12_utilities.hpp>

#include "Render_data.hpp"

using namespace Maia::Renderer::D3D12;

namespace Maia::Mythology::D3D12
{
	Frames_resources::Frames_resources(ID3D12Device& device, UINT pipeline_length) :
		rtv_descriptor_heap{ create_descriptor_heap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, pipeline_length, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0) }
	{
	}

	Render_resources::Render_resources(IDXGIAdapter4& adapter, std::uint8_t const pipeline_length) :
		device{ create_device(adapter, D3D_FEATURE_LEVEL_11_0) },
		direct_command_queue{ create_command_queue(*device, D3D12_COMMAND_LIST_TYPE_DIRECT, 0, D3D12_COMMAND_QUEUE_FLAG_NONE, 0) },
		command_allocators{ create_command_allocators(*device, D3D12_COMMAND_LIST_TYPE_DIRECT, pipeline_length) },
		command_list{ create_opened_graphics_command_list(*device, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, *command_allocators[0], nullptr) },
		upload_heap{ create_upload_heap(*device, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT) },
		upload_buffer{ create_buffer(*device, *upload_heap, 0, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, D3D12_RESOURCE_STATE_GENERIC_READ) },
		upload_buffer_offset{ 0 },
		buffers_heap{ create_buffer_heap(*device, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT * 2) },
		buffers_heap_offset{ 0 }
	{
	}
}
