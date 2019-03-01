#include <Maia/GameEngine/Entity_type.hpp>
#include <Maia/GameEngine/Entity_manager.hpp>
#include <Maia/GameEngine/Systems/Transform_system.hpp>

#include <Maia/Renderer/D3D12/Utilities/Check_hresult.hpp>
#include <Maia/Renderer/D3D12/Utilities/D3D12_utilities.hpp>
#include <Maia/Renderer/Matrices.hpp>

#include <Render/Pass_data.hpp>

#include "Upload_frame_data_system.hpp"

using namespace Maia::Renderer::D3D12;

namespace Maia::Mythology::D3D12
{
	constexpr UINT64 c_upload_buffer_offset_per_frame = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

	Upload_frame_data_system::Upload_frame_data_system(ID3D12Device& device, std::uint8_t const pipeline_length) :
		m_command_allocators{ create_command_allocators(device, D3D12_COMMAND_LIST_TYPE_COPY, pipeline_length) },
		m_command_list{ create_closed_graphics_command_list(device, 0, D3D12_COMMAND_LIST_TYPE_COPY, *m_command_allocators.front()) },
		m_upload_heap{ create_upload_heap(device, c_upload_buffer_offset_per_frame * pipeline_length) },
		m_upload_buffer{ create_buffer(device, *m_upload_heap, 0, c_upload_buffer_offset_per_frame * pipeline_length, D3D12_RESOURCE_STATE_GENERIC_READ) }
	{
	}


	Upload_bundle Upload_frame_data_system::reset(std::uint8_t const frame_index)
	{
		ID3D12CommandAllocator& command_allocator = *m_command_allocators[frame_index];

		check_hresult(
			command_allocator.Reset());

		check_hresult(
			m_command_list->Reset(&command_allocator, nullptr));

		return { frame_index, frame_index * c_upload_buffer_offset_per_frame };
	}


	namespace
	{
		std::vector<D3D12_VERTEX_BUFFER_VIEW> upload_instance_data_impl(
			Instance_buffer const& instance_buffer,
			Maia::GameEngine::Entity_manager const& entity_manager,
			gsl::span<Maia::GameEngine::Entity_type_id const> entity_types_ids,
			ID3D12GraphicsCommandList& command_list,
			ID3D12Resource& upload_buffer, UINT64 const upload_buffer_offset_in_bytes,
			UINT64& uploaded_size_in_bytes
		)
		{
			using namespace Maia::GameEngine;
			using namespace Maia::Renderer::D3D12;

			std::vector<D3D12_VERTEX_BUFFER_VIEW> instance_buffer_views;
			instance_buffer_views.reserve(entity_types_ids.size());

			UINT64 instance_buffer_offset_in_bytes{ 0 };

			for (Entity_type_id const entity_type_id : entity_types_ids)
			{
				Component_group const& component_group = entity_manager.get_component_group(entity_type_id);

				UINT64 size_in_bytes{ 0 };

				for (std::size_t chunk_index = 0; chunk_index < component_group.num_chunks(); ++chunk_index)
				{
					using namespace Maia::GameEngine::Systems;

					gsl::span<Transform_matrix const> const transform_matrices =
						component_group.components<Transform_matrix>(chunk_index);

					upload_buffer_data(
						command_list,
						*instance_buffer.value, instance_buffer_offset_in_bytes + size_in_bytes,
						upload_buffer, upload_buffer_offset_in_bytes + instance_buffer_offset_in_bytes + size_in_bytes,
						transform_matrices
					);

					size_in_bytes += transform_matrices.size_bytes();
				}

				D3D12_VERTEX_BUFFER_VIEW instance_buffer_view;
				instance_buffer_view.BufferLocation =
					instance_buffer.value->GetGPUVirtualAddress() +
					instance_buffer_offset_in_bytes;
				instance_buffer_view.SizeInBytes = static_cast<UINT>(size_in_bytes);
				instance_buffer_view.StrideInBytes = sizeof(Instance_data);
				instance_buffer_views.push_back(instance_buffer_view);

				instance_buffer_offset_in_bytes += size_in_bytes;
			}

			uploaded_size_in_bytes = instance_buffer_offset_in_bytes;

			return instance_buffer_views;
		}
	}

	std::vector<D3D12_VERTEX_BUFFER_VIEW> Upload_frame_data_system::upload_instance_data(
		Upload_bundle& bundle,
		Instance_buffer const& instance_buffer,
		Maia::GameEngine::Entity_manager const& entity_manager,
		gsl::span<Maia::GameEngine::Entity_type_id const> const entity_types_ids
	)
	{
		UINT64 uploaded_size_in_bytes;

		std::vector<D3D12_VERTEX_BUFFER_VIEW> vertex_buffer_views = upload_instance_data_impl(
			instance_buffer,
			entity_manager, entity_types_ids,
			*m_command_list,
			*m_upload_buffer, bundle.offset,
			uploaded_size_in_bytes
		);
		
		bundle.offset += uploaded_size_in_bytes;

		return vertex_buffer_views;
	}

	namespace
	{
		Eigen::Matrix4f create_api_specific_matrix()
		{
			Eigen::Matrix4f value;
			value <<
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, -1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f;
			return value;
		}

		UINT64 upload_pass_data_impl(
			Camera const& camera,
			ID3D12GraphicsCommandList& command_list,
			ID3D12Resource& destination_buffer, UINT64 const destination_buffer_offset,
			ID3D12Resource& upload_buffer, UINT64 const upload_buffer_offset
		)
		{
			using namespace Maia::Renderer::D3D12;
			
			Pass_data pass_data;
			pass_data.view_matrix = Maia::Renderer::create_view_matrix(camera.position.value, camera.rotation.value);
			pass_data.projection_matrix =
				create_api_specific_matrix() *
				Maia::Renderer::create_perspective_projection_matrix(camera.vertical_half_angle_of_view, camera.width_by_height_ratio, camera.zRange);

			upload_buffer_data<Pass_data>(
				command_list,
				destination_buffer, destination_buffer_offset,
				upload_buffer, upload_buffer_offset,
				{ &pass_data, 1 }
			);

			return sizeof(Pass_data);
		}
	}

	void Upload_frame_data_system::upload_pass_data(
		Upload_bundle& bundle,
		Camera camera, 
		ID3D12Resource& pass_buffer, UINT64 pass_buffer_offset
	)
	{
		UINT64 const uploaded_size_in_bytes = upload_pass_data_impl(
			camera,
			*m_command_list,
			pass_buffer, pass_buffer_offset,
			*m_upload_buffer, bundle.offset
		);

		bundle.offset += uploaded_size_in_bytes;
	}


	ID3D12CommandList& Upload_frame_data_system::close(Upload_bundle& bundle)
	{
		assert(bundle.offset < (bundle.frame_index + 1) * c_upload_buffer_offset_per_frame);

		check_hresult(
			m_command_list->Close());

		return *m_command_list;
	}
}
