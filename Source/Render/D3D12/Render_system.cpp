#include <iostream>

#include <Maia/Renderer/D3D12/Utilities/D3D12_utilities.hpp>

#include "Render_system.hpp"

namespace Maia::Mythology::D3D12
{
	namespace
	{
		winrt::com_ptr<IDXGIAdapter4> select_adapter(IDXGIFactory6& factory)
		{
			winrt::com_ptr<IDXGIAdapter4> adapter = Maia::Renderer::D3D12::select_adapter(factory, false);

			{
				DXGI_ADAPTER_DESC3 description;
				winrt::check_hresult(
					adapter->GetDesc3(&description));

				std::wcout << std::wstring_view{ description.Description } << '\n';
			}

			return adapter;
		}
	}

	Render_system::Render_system(Window const& window) :
		m_factory{ Maia::Renderer::D3D12::create_factory({}) },
		m_adapter{ select_adapter(*m_factory) },
		m_pipeline_length{ 3 },
		m_render_resources{ *m_adapter, m_pipeline_length },
		m_renderer{ *m_factory, m_render_resources, window.bounds, m_pipeline_length },
		m_frames_resources{ *m_render_resources.device, m_pipeline_length },
		m_window_swap_chain{ *m_factory, *m_render_resources.direct_command_queue, window.value, *m_render_resources.device, m_frames_resources.rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart() }
	{
		/*m_scene_resources = Maia::Mythology::load(m_entity_manager, *m_render_resources);
		m_scene_resources.camera.width_by_height_ratio = bounds.Width / bounds.Height;*/
	}

	void Render_system::render_frame(Maia::GameEngine::Entity_manager const& entity_manager)
	{
		// TODO upload instance data
		{
			using namespace Maia::Mythology;

			ID3D12Device& device = *m_render_resources.device;
			ID3D12Heap& heap = *m_render_resources.buffers_heap;
			UINT64 const heap_offset = m_render_resources.buffers_heap_offset;
			D3D12::Scene_resources& scene_resources = m_scene_resources;

			scene_resources.instance_buffers.reserve(scene_resources.instances_count.size());
			scene_resources.dirty_instance_buffers.reserve(scene_resources.instances_count.size());

			UINT allocated_bytes{ 0 };

			for (UINT const instance_count : scene_resources.instances_count)
			{
				D3D12::Instance_buffer instance_buffer{};

				instance_buffer.value = Maia::Renderer::D3D12::create_buffer(
					device, heap, heap_offset + allocated_bytes, instance_count * sizeof(Instance_data));

				allocated_bytes += instance_count * sizeof(Instance_data);
				scene_resources.instance_buffers.push_back(instance_buffer);
				scene_resources.dirty_instance_buffers.push_back(true);
			}

			m_render_resources.buffers_heap_offset += allocated_bytes;

			// TODO move this to initialize of render system
			// TODO upload data on render system
		}

		{
			IDXGISwapChain4& swap_chain = m_window_swap_chain.get();

			UINT const back_buffer_index = swap_chain.GetCurrentBackBufferIndex();
			winrt::com_ptr<ID3D12Resource> back_buffer;
			winrt::check_hresult(
				swap_chain.GetBuffer(back_buffer_index, __uuidof(back_buffer), back_buffer.put_void()));

			UINT const descriptor_handle_increment_size =
				m_render_resources.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

			D3D12_CPU_DESCRIPTOR_HANDLE const render_target_descriptor_handle
			{
				m_frames_resources.rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart().ptr +
					descriptor_handle_increment_size * back_buffer_index
			};

			m_renderer.render(*back_buffer, render_target_descriptor_handle, m_scene_resources);

			m_window_swap_chain.present();
		}
	}

	void Render_system::on_window_resized(Eigen::Vector2i new_size)
	{
		m_renderer.wait();

		m_window_swap_chain.resize(
			*m_render_resources.direct_command_queue,
			new_size,
			*m_render_resources.device,
			m_frames_resources.rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart()
		);

		m_renderer.resize_viewport_and_scissor_rects(new_size);
		
		//m_scene_resources.camera.width_by_height_ratio = size.Width / size.Height;
	}
}
