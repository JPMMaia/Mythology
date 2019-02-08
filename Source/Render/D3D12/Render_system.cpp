#include <iostream>

#include <Maia/GameEngine/Component_group.hpp>
#include <Maia/GameEngine/Entity_manager.hpp>
#include <Maia/GameEngine/Entity_type.hpp>

#include <Maia/Renderer/D3D12/Utilities/D3D12_utilities.hpp>

#include <Render/Pass_data.hpp>

#include "Render_system.hpp"


using namespace Maia::Renderer::D3D12;

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

		std::size_t calculate_instance_buffer_size(
			Maia::GameEngine::Entity_manager const& entity_manager,
			gsl::span<Maia::GameEngine::Entity_type_id const> const entity_types_ids
		)
		{
			using namespace Maia::GameEngine;

			std::size_t count{ 0 };

			for (Maia::GameEngine::Entity_type_id const entity_type_id : entity_types_ids)
			{
				Component_group const& component_group = entity_manager.get_component_group(entity_type_id);

				count += component_group.size();
			}

			std::size_t const unaligned_size_in_bytes = count * sizeof(Instance_data);
			std::size_t constexpr alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

			std::size_t const num_blocks = unaligned_size_in_bytes / alignment
				+ (unaligned_size_in_bytes % alignment == 0 ? 0 : 1);

			return num_blocks * alignment;
		}

		std::vector<Instance_buffer> create_instance_buffers(
			ID3D12Device& device,
			ID3D12Heap& heap, UINT64 const heap_offset,
			UINT64 const size,
			std::size_t const count
		)
		{
			std::vector<Instance_buffer> instance_buffers;
			instance_buffers.reserve(count);

			for (std::size_t index = 0; index < count; ++index)
			{
				const UINT64 current_heap_offset = heap_offset + 
					index * size;

				instance_buffers.push_back(
					{
						create_buffer(
							device, 
							heap, current_heap_offset, 
							size,
							D3D12_RESOURCE_STATE_COPY_DEST
						)
					}
				);
			}

			return instance_buffers;
		}
	}

	Render_system::Render_system(Window const& window) :
		m_factory{ Maia::Renderer::D3D12::create_factory({}) },
		m_adapter{ select_adapter(*m_factory) },
		m_render_resources{ *m_adapter, m_pipeline_length },
		m_copy_command_queue{ create_command_queue(*m_render_resources.device, D3D12_COMMAND_LIST_TYPE_COPY, 0, D3D12_COMMAND_QUEUE_FLAG_NONE, 0) },
		m_direct_command_queue{ create_command_queue(*m_render_resources.device, D3D12_COMMAND_LIST_TYPE_DIRECT, 0, D3D12_COMMAND_QUEUE_FLAG_NONE, 0) },
		m_upload_frame_data_system{ *m_render_resources.device, m_pipeline_length },
		m_renderer{ *m_factory, *m_render_resources.device, window.bounds, m_pipeline_length },
		m_window_swap_chain{ *m_factory, *m_render_resources.direct_command_queue, window.value, *m_render_resources.device, m_frames_resources.rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart() },
		m_frames_resources{ *m_render_resources.device, m_pipeline_length },

		m_pipeline_length{ 3 },
		m_fence_value{ 0 },
		m_fence{ create_fence(*m_render_resources.device, m_fence_value, D3D12_FENCE_FLAG_NONE) },
		m_fence_event{ ::CreateEvent(nullptr, false, false, nullptr) },
		m_submitted_frames{ m_pipeline_length },

		m_pass_heap{ create_buffer_heap(*m_render_resources.device, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT) },
		m_pass_buffer{ create_buffer(*m_render_resources.device, *m_pass_heap, 0, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, D3D12_RESOURCE_STATE_COPY_DEST) },

		m_instance_buffers_heap{ create_buffer_heap(*m_render_resources.device, m_pipeline_length * D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT) },
		m_instance_buffer_per_frame{ create_instance_buffers(*m_render_resources.device, *m_instance_buffers_heap, 0, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, m_pipeline_length) }
	{
		// TODO maybe execute command list?

		/*m_scene_resources = Maia::Mythology::load(m_entity_manager, *m_render_resources);
		m_scene_resources.camera.width_by_height_ratio = bounds.Width / bounds.Height;*/

		/*
		ID3D12GraphicsCommandList& command_list = *render_resources.command_list;
		winrt::check_hresult(
			command_list.Close());

		ID3D12CommandQueue& command_queue = *render_resources.direct_command_queue;
		{
			std::array<ID3D12CommandList*, 1> command_lists_to_execute
			{
				&command_list
			};

			command_queue.ExecuteCommandLists(
				static_cast<UINT>(command_lists_to_execute.size()), command_lists_to_execute.data()
			);
		}

		{
			UINT64 constexpr event_value_to_signal_and_wait = 1;
			signal_and_wait(command_queue, *m_fence, m_fence_event.get(), event_value_to_signal_and_wait, INFINITE);
		}

		*/
	}

	void Render_system::render_frame(
		Camera camera,
		Maia::GameEngine::Entity_manager const& entity_manager,
		gsl::span<Maia::GameEngine::Entity_type_id const> const entity_types_ids,
		gsl::span<Maia::Mythology::D3D12::Mesh_view const> const mesh_views
	)
	{
		std::uint8_t const current_frame_index{ m_submitted_frames % m_pipeline_length };

		{
			// TODO check if it is needed to create new instance buffers
		}

		{
			// TODO return to do other cpu work instead of waiting

			ID3D12CommandQueue& direct_command_queue = *m_direct_command_queue;

			UINT64 const event_value_to_wait = m_submitted_frames - m_pipeline_length;
			Maia::Renderer::D3D12::wait(
				direct_command_queue, *m_fence, m_fence_event.get(), event_value_to_wait, INFINITE);
		}

		std::vector<D3D12_VERTEX_BUFFER_VIEW> instance_buffer_views;
		{
			m_upload_frame_data_system.reset(current_frame_index);


			ID3D12Resource& pass_buffer = *m_pass_buffer;
			m_upload_frame_data_system.upload_pass_data(
				camera,
				pass_buffer, current_frame_index * sizeof(Pass_data)
			);


			Instance_buffer const& instance_buffer =
				m_instance_buffer_per_frame[current_frame_index];

			instance_buffer_views =
				m_upload_frame_data_system.upload_instance_data(
					instance_buffer,
					entity_manager,
					entity_types_ids
				);

			{
				ID3D12CommandList& command_list =
					m_upload_frame_data_system.close();

				std::array<ID3D12CommandList*, 1> command_lists_to_execute
				{
					&command_list
				};

				ID3D12CommandQueue& command_queue = *m_copy_command_queue;
				command_queue.ExecuteCommandLists(
					static_cast<UINT>(command_lists_to_execute.size()), command_lists_to_execute.data()
				);
			}

			// TODO Sync copy queue with other render queue
			// TODO signal
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

			{
				D3D12_GPU_VIRTUAL_ADDRESS const pass_data_buffer_address =
					m_pass_buffer->GetGPUVirtualAddress() + current_frame_index * sizeof(Pass_data);

				ID3D12CommandList& command_list =
					m_renderer.render(
						current_frame_index,
						*back_buffer, render_target_descriptor_handle,
						mesh_views,
						instance_buffer_views,
						pass_data_buffer_address
					);

				std::array<ID3D12CommandList*, 1> command_lists_to_execute
				{
					&command_list
				};

				// TODO wait for copy queue signal
				ID3D12CommandQueue& command_queue = *m_direct_command_queue;
				command_queue.ExecuteCommandLists(
					static_cast<UINT>(command_lists_to_execute.size()), command_lists_to_execute.data()
				);
			}

			{
				UINT64 const frame_finished_value = static_cast<UINT64>(m_submitted_frames);

				ID3D12CommandQueue& command_queue = *m_direct_command_queue;
				command_queue.Signal(m_fence.get(), frame_finished_value);
				++m_submitted_frames;
			}

			m_window_swap_chain.present();
		}
	}

	void Render_system::wait()
	{
		ID3D12CommandQueue& command_queue = *m_render_resources.direct_command_queue;

		UINT64 const event_value_to_signal_and_wait = m_submitted_frames + m_pipeline_length;
		signal_and_wait(command_queue, *m_fence, m_fence_event.get(), event_value_to_signal_and_wait, INFINITE);
	}

	void Render_system::on_window_resized(Eigen::Vector2i new_size)
	{
		wait();

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
