#include <iostream>

#include <Maia/GameEngine/Component_group.hpp>
#include <Maia/GameEngine/Entity_manager.hpp>
#include <Maia/GameEngine/Entity_type.hpp>

#include <Maia/Renderer/D3D12/Utilities/Check_hresult.hpp>
#include <Maia/Renderer/D3D12/Utilities/D3D12_utilities.hpp>
#include <Maia/Renderer/Matrices.hpp>

#include <Components/Camera_component.hpp>
#include <Render/Pass_data.hpp>

#include "Render_system.hpp"


using namespace Maia::Renderer::D3D12;

namespace Maia::Mythology::D3D12
{
	namespace
	{
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
							D3D12_RESOURCE_STATE_COMMON
						)
					}
				);
			}

			return instance_buffers;
		}
	}

	Render_system::Render_system(
		ID3D12Device& device,
		ID3D12CommandQueue& copy_command_queue,
		ID3D12CommandQueue& direct_command_queue,
		Swap_chain swap_chain,
		std::uint8_t const pipeline_length,
		bool const vertical_sync
	) :
		m_device{ device },
		m_copy_command_queue{ copy_command_queue },
		m_direct_command_queue{ direct_command_queue },
		m_swap_chain{ swap_chain.value },
		m_pipeline_length{ pipeline_length },
		m_vertical_sync{ vertical_sync },
		m_copy_fence_value{ 0 },
		m_copy_fence{ create_fence(device, m_copy_fence_value, D3D12_FENCE_FLAG_NONE) },

		m_upload_frame_data_system{ device, m_pipeline_length },
		m_renderer{ device, swap_chain.bounds, m_pipeline_length },
		m_frames_resources{ device, m_pipeline_length },

		m_submitted_frames{ 0 },
		m_fence{ create_fence(device, m_submitted_frames, D3D12_FENCE_FLAG_NONE) },
		m_fence_event{ ::CreateEvent(nullptr, false, false, nullptr) },

		m_pass_heap{ create_buffer_heap(device, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT) },
		m_pass_buffer{ create_buffer(device, *m_pass_heap, 0, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, D3D12_RESOURCE_STATE_COMMON) },

		m_instance_buffers_heap{ create_buffer_heap(device, m_pipeline_length * D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT) },
		m_instance_buffer_per_frame{ create_instance_buffers(device, *m_instance_buffers_heap, 0, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, m_pipeline_length) }
	{
		create_swap_chain_rtvs(
			device,
			m_swap_chain,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			m_frames_resources.rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(),
			m_pipeline_length
		);

		/*m_scene_resources = Maia::Mythology::load(m_entity_manager, *m_render_resources);
		m_scene_resources.camera.width_by_height_ratio = bounds.Width / bounds.Height;*/

		/*
		ID3D12GraphicsCommandList& command_list = *render_resources.command_list;
		check_hresult(
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

	namespace
	{
		template <class T, class U>
		T align(T value, U alignment)
		{
			return ((value - 1) | (alignment - 1)) + 1;
		}

		Eigen::Matrix4f to_api_specific_perspective_matrix()
		{
			Eigen::Matrix4f value;
			value <<
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, -1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f;
			return value;
		}
	}

	void Render_system::render_frame(
		Maia::GameEngine::Entity_manager const& entity_manager,
		Maia::GameEngine::Entity const camera_entity,
		gsl::span<Maia::GameEngine::Entity_type_id const> const entity_types_ids,
		gsl::span<Maia::Mythology::D3D12::Mesh_view const> const mesh_views
	)
	{
		std::uint8_t const current_frame_index{ m_submitted_frames % m_pipeline_length };

		{
			// TODO check if it is needed to create new instance buffers
		}

		if (m_submitted_frames >= m_pipeline_length)
		{
			// TODO return to do other cpu work instead of waiting

			UINT64 const event_value_to_wait = m_submitted_frames - m_pipeline_length + 1;

			Maia::Renderer::D3D12::wait(
				m_direct_command_queue, *m_fence, m_fence_event.get(), event_value_to_wait, INFINITE);
		}

		std::vector<D3D12_VERTEX_BUFFER_VIEW> instance_buffer_views;
		{
			Upload_bundle bundle =
				m_upload_frame_data_system.reset(current_frame_index);


			{
				using namespace Maia::Renderer;
				using namespace Maia::Utilities::glTF;

				Maia::Utilities::glTF::Camera const camera =
					entity_manager.get_component_data<Camera_component>(camera_entity).value;

				Pass_data pass_data;
				pass_data.view_matrix;

				if (camera.type == Maia::Utilities::glTF::Camera::Type::Orthographic)
				{
					const auto& orthographic = std::get<Maia::Utilities::glTF::Camera::Orthographic>(camera.projection);

					pass_data.projection_matrix =
						to_api_specific_perspective_matrix() *
						create_orthographic_projection_matrix(
							orthographic.horizontal_magnification,
							orthographic.vertical_magnification,
							orthographic.near_z,
							orthographic.far_z
						);
				}
				else
				{
					const auto& perspective = std::get<Maia::Utilities::glTF::Camera::Perspective>(camera.projection);

					if (perspective.far_z)
					{
						// TODO finite perspective
					}
					else
					{
						// TODO infinite perspective
					}
				}

				pass_data.projection_matrix =
					Maia::Renderer::create_perspective_projection_matrix;

				ID3D12Resource& pass_buffer = *m_pass_buffer;
				m_upload_frame_data_system.upload_pass_data(
					bundle,
					pass_data,
					pass_buffer, current_frame_index * align(sizeof(Pass_data), 256)
				);
			}


			Instance_buffer const& instance_buffer =
				m_instance_buffer_per_frame[current_frame_index];

			instance_buffer_views =
				m_upload_frame_data_system.upload_instance_data(
					bundle,
					instance_buffer,
					entity_manager,
					entity_types_ids
				);

			{
				ID3D12CommandList& command_list =
					m_upload_frame_data_system.close(bundle);

				std::array<ID3D12CommandList*, 1> command_lists_to_execute
				{
					&command_list
				};

				m_copy_command_queue.ExecuteCommandLists(
					static_cast<UINT>(command_lists_to_execute.size()), command_lists_to_execute.data()
				);
			}
		}

		{
			UINT64 const copy_fence_value_to_signal_and_wait = ++m_copy_fence_value;
			m_copy_command_queue.Signal(m_copy_fence.get(), copy_fence_value_to_signal_and_wait);
			m_direct_command_queue.Wait(m_copy_fence.get(), copy_fence_value_to_signal_and_wait);
		}

		{
			IDXGISwapChain3& swap_chain = m_swap_chain;

			UINT const back_buffer_index = swap_chain.GetCurrentBackBufferIndex();
			winrt::com_ptr<ID3D12Resource> back_buffer;
			check_hresult(
				swap_chain.GetBuffer(back_buffer_index, __uuidof(back_buffer), back_buffer.put_void()));

			UINT const descriptor_handle_increment_size =
				m_device.GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

			D3D12_CPU_DESCRIPTOR_HANDLE const render_target_descriptor_handle
			{
				m_frames_resources.rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart().ptr +
					descriptor_handle_increment_size * back_buffer_index
			};

			{
				D3D12_GPU_VIRTUAL_ADDRESS const pass_data_buffer_address =
					m_pass_buffer->GetGPUVirtualAddress() + current_frame_index * align(sizeof(Pass_data), 256);

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

				m_direct_command_queue.ExecuteCommandLists(
					static_cast<UINT>(command_lists_to_execute.size()), command_lists_to_execute.data()
				);
			}

			{
				UINT const sync_interval =
					m_vertical_sync ? 1 : 0;

				check_hresult(
					m_swap_chain.Present(sync_interval, 0));
			}

			{
				UINT64 const frame_finished_value = ++m_submitted_frames;

				check_hresult(
					m_direct_command_queue.Signal(m_fence.get(), frame_finished_value));
			}
		}
	}

	void Render_system::wait()
	{
		UINT64 const event_value_to_signal_and_wait = m_submitted_frames + m_pipeline_length;
		signal_and_wait(m_direct_command_queue, *m_fence, m_fence_event.get(), event_value_to_signal_and_wait, INFINITE);
	}

	void Render_system::on_window_resized(Eigen::Vector2i new_size)
	{
		wait();

		{
			ID3D12CommandQueue& command_queue = m_direct_command_queue;

			std::vector<UINT> create_node_masks(m_pipeline_length, 1);
			std::vector<IUnknown*> command_queues(m_pipeline_length, &command_queue);

			resize_swap_chain_buffers_and_recreate_rtvs(
				m_swap_chain,
				create_node_masks,
				command_queues,
				new_size,
				m_device,
				m_frames_resources.rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart()
			);
		}

		m_renderer.resize_viewport_and_scissor_rects(new_size);

		//m_scene_resources.camera.width_by_height_ratio = size.Width / size.Height;
	}
}
