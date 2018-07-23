#include <Maia/Renderer/D3D12/Utilities/D3D12_utilities.h>

#include <D3D12_renderer.h>

using namespace Maia::Renderer::D3D12;

namespace Mythology
{
	D3D12_renderer::D3D12_renderer() :
		m_factory(create_factory({})),
		m_adapter(select_adapter(*m_factory, false)),
		m_device(create_device(*m_adapter, D3D_FEATURE_LEVEL_12_1)),
		m_render_command_queue(create_command_queue(*m_device, D3D12_COMMAND_LIST_TYPE_DIRECT, 0, D3D12_COMMAND_QUEUE_FLAG_NONE, 0)),
		m_command_allocator(create_command_allocator(*m_device, D3D12_COMMAND_LIST_TYPE_DIRECT)),
		m_command_list(create_closed_graphics_command_list(*m_device, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, *m_command_allocator, nullptr)),
		m_descriptor_heap(),
		m_fence(),
		m_fence_event(),
		m_fence_value(0)
	{
	}
}
