#include <array>

#include <Eigen/Core>

#include <d3d12.h>

#include <winrt/base.h>

#include <Maia/Renderer/Matrices.hpp>
#include <Maia/Renderer/D3D12/Utilities/D3D12_utilities.hpp>

#include <Maia/GameEngine/Systems/Transform_system.hpp>

#include "Renderer/Pass_data.hpp"
#include "Scene.hpp"

namespace Maia::Mythology
{
	struct Position
	{
		Eigen::Vector4f value;
	};

	struct Color
	{
		Eigen::Vector4f value;
	};

	struct Colored_mesh
	{
		struct Vertex_data
		{
			std::vector<Position> positions;
			std::vector<Color> colors;
		};

		Vertex_data vertices;
		std::vector<std::uint16_t> indices;
	};

	struct Instance_data
	{
		Eigen::Matrix4f world_matrix;
	};

	namespace
	{
		Colored_mesh create_triangle_mesh()
		{
			std::vector<Position> vertices_positions
			{
				Position
				{ { -1.0f, 1.0f, 0.0f, 1.0f } },
				{ { 1.0f, 1.0f, 0.0f, 1.0f } },
				{ { 0.0f, -1.0f, 0.0f, 1.0f } },
			};

			std::vector<Color> vertices_colors
			{
				Color
				{ { 1.0f, 0.0f, 0.0f, 1.0f } },
				{ { 0.0f, 1.0f, 0.0f, 1.0f } },
				{ { 0.0f, 0.0f, 1.0f, 1.0f } },
			};

			std::vector<std::uint16_t> indices
			{
				0, 1, 2
			};

			return { { vertices_positions, vertices_colors }, indices };
		}

		void upload_geometry_data(
			Colored_mesh const& mesh,
			D3D12::Geometry_and_instances_buffer& buffer, UINT64 const base_buffer_offset,
			ID3D12Resource& upload_buffer, UINT64 const upload_buffer_offset, // TODO create a struct for both
			ID3D12GraphicsCommandList& command_list
		)
		{
			using namespace Maia::Renderer::D3D12;

			Mapped_memory mapped_memory{ upload_buffer, 0, {} };

			UINT64 buffer_offset{ base_buffer_offset };

			{
				gsl::span<Position const> const positions{ mesh.vertices.positions };
				upload_buffer_data(
					mapped_memory,
					command_list,
					*buffer.value, buffer_offset,
					upload_buffer, upload_buffer_offset + buffer_offset,
					positions
				);
				buffer_offset += positions.size_bytes();
			}

			{
				gsl::span<Color const> const colors{ mesh.vertices.colors };
				upload_buffer_data(
					mapped_memory,
					command_list,
					*buffer.value, buffer_offset,
					upload_buffer, upload_buffer_offset + buffer_offset,
					colors
				);
				buffer_offset += colors.size_bytes();
			}

			{
				gsl::span<decltype(mesh.indices)::value_type const> const indices{ mesh.indices };
				upload_buffer_data(
					mapped_memory,
					command_list,
					*buffer.value, buffer_offset,
					upload_buffer, upload_buffer_offset + buffer_offset,
					indices
				);
				buffer_offset += indices.size_bytes();
			}
		}

		void upload_instances_data(
			D3D12::Geometry_and_instances_buffer& buffer, UINT64 const base_buffer_offset,
			ID3D12Resource& upload_buffer, UINT64 const upload_buffer_offset, // TODO create a struct for both
			ID3D12GraphicsCommandList& command_list,
			gsl::span<Instance_data const> instances_data)
		{
			using namespace Maia::Renderer::D3D12;

			upload_buffer_data(
				command_list,
				*buffer.value, base_buffer_offset,
				upload_buffer, upload_buffer_offset,
				instances_data
			);
		}
	}

	Maia::Mythology::D3D12::Scene_resources load(Maia::GameEngine::Entity_manager& entity_manager, Maia::Mythology::D3D12::Render_resources const& render_resources)
	{
		ID3D12Device& device = *render_resources.device;
		ID3D12Heap& heap = *render_resources.buffers_heap;
		UINT64 const heap_offset = render_resources.buffers_heap_offset;
		ID3D12Resource& upload_buffer = *render_resources.upload_buffer;
		UINT64 const upload_buffer_offset = render_resources.upload_buffer_offset;
		ID3D12GraphicsCommandList& command_list = *render_resources.command_list;

		using namespace Maia::Renderer::D3D12;

		Maia::Mythology::D3D12::Scene_resources scene_resources;

		{
			using namespace Maia::GameEngine;
			using namespace Maia::GameEngine::Systems;
		
			Entity_type<Transform_matrix> triangle_entity_type =
				entity_manager.create_entity_type<Transform_matrix>(100);

			constexpr std::size_t instance_count = 100;
			{
				float const z = 10.0f;
				for (std::size_t i = 0; i < 10; ++i)
				{
					float const y = -15.0f + i * 3.0f;

					for (std::size_t j = 0; j < 10; ++j)
					{
						float const x = -15.0f + j * 3.0f;

						Eigen::Matrix4f world_matrix;
						world_matrix <<
							1.0f, 0.0f, 0.0f, x,
							0.0f, 1.0f, 0.0f, y,
							0.0f, 0.0f, 1.0f, z,
							0.0f, 0.0f, 0.0f, 1.0f;
						
						entity_manager.create_entity(triangle_entity_type, Transform_matrix{world_matrix});
					}
				}
			}

			// Create geometry and instances buffer
			{
				Colored_mesh const mesh = create_triangle_mesh();

				UINT64 positions_size_bytes = mesh.vertices.positions.size() * sizeof(decltype(mesh.vertices.positions)::value_type);
				UINT64 color_size_bytes = mesh.vertices.colors.size() * sizeof(decltype(mesh.vertices.colors)::value_type);
				UINT64 indices_size_bytes = mesh.indices.size() * sizeof(decltype(mesh.indices)::value_type);
				UINT64 const geometry_buffer_width =
					positions_size_bytes + color_size_bytes + indices_size_bytes;
				UINT64 const instances_data_size_bytes = sizeof(Instance_data) * instance_count;
				UINT64 const total_buffer_width = geometry_buffer_width + instances_data_size_bytes;

				{
					D3D12::Geometry_and_instances_buffer geometry_and_instances_buffer =
					{
						create_buffer(
							device,
							heap, heap_offset,
							total_buffer_width,
							D3D12_RESOURCE_STATE_COPY_DEST
						)
					};

					upload_geometry_data(mesh, geometry_and_instances_buffer, 0, upload_buffer, upload_buffer_offset, command_list);

					scene_resources.geometry_and_instances_buffers.emplace_back(
						std::move(geometry_and_instances_buffer)
					);
				}

				{
					D3D12::Geometry_and_instances_buffer& buffer = scene_resources.geometry_and_instances_buffers.back();

					Component_group const& component_group = entity_manager.get_component_group(triangle_entity_type.id);
					
					gsl::span<Transform_matrix const> transform_matrices = component_group.components<Transform_matrix>(0);
					gsl::span<Instance_data const> instances_data{ reinterpret_cast<Instance_data const*>(transform_matrices.data()), transform_matrices.size() };
					upload_instances_data(buffer, geometry_buffer_width, upload_buffer, upload_buffer_offset + geometry_buffer_width, command_list, instances_data);
				}

				{
					Maia::Mythology::D3D12::Render_primitive render_primitive{};

					D3D12::Geometry_and_instances_buffer& buffer = scene_resources.geometry_and_instances_buffers.back();
					D3D12_GPU_VIRTUAL_ADDRESS const geometry_and_instances_buffer_address = 
						buffer.value->GetGPUVirtualAddress();
					UINT const instances_data_size_bytes{ instance_count * sizeof(Instance_data) };

					{
						D3D12_VERTEX_BUFFER_VIEW const position_buffer_view = [&]() -> D3D12_VERTEX_BUFFER_VIEW
						{
							D3D12_VERTEX_BUFFER_VIEW buffer_view;
							buffer_view.BufferLocation = geometry_and_instances_buffer_address;

							using data_type = decltype(mesh.vertices.positions)::value_type;
							gsl::span<data_type const> const data_view = mesh.vertices.positions;
							buffer_view.SizeInBytes = static_cast<UINT>(data_view.size_bytes());
							buffer_view.StrideInBytes = sizeof(data_type);

							return buffer_view;
						}();

						D3D12_VERTEX_BUFFER_VIEW const color_buffer_view = [&]() -> D3D12_VERTEX_BUFFER_VIEW
						{
							D3D12_VERTEX_BUFFER_VIEW buffer_view;
							buffer_view.BufferLocation = geometry_and_instances_buffer_address +
								positions_size_bytes;

							using data_type = decltype(mesh.vertices.colors)::value_type;
							gsl::span<data_type const> const data_view = mesh.vertices.colors;
							buffer_view.SizeInBytes = static_cast<UINT>(data_view.size_bytes());
							buffer_view.StrideInBytes = sizeof(data_type);

							return buffer_view;
						}();

						D3D12_VERTEX_BUFFER_VIEW const instance_buffer_view = [&]() -> D3D12_VERTEX_BUFFER_VIEW
						{
							D3D12_VERTEX_BUFFER_VIEW buffer_view;
							buffer_view.BufferLocation = geometry_and_instances_buffer_address +
								geometry_buffer_width;
							buffer_view.SizeInBytes = instances_data_size_bytes;
							buffer_view.StrideInBytes = sizeof(Instance_data);

							return buffer_view;
						}();

						render_primitive.vertex_buffer_views =
						{
							position_buffer_view,
							color_buffer_view,
							instance_buffer_view
						};
					}

					{
						D3D12_INDEX_BUFFER_VIEW index_buffer_view;
						index_buffer_view.BufferLocation = geometry_and_instances_buffer_address +
							positions_size_bytes + color_size_bytes;

						using data_type = decltype(mesh.indices)::value_type;
						gsl::span<data_type const> const data_view = mesh.indices;
						index_buffer_view.SizeInBytes = static_cast<UINT>(data_view.size_bytes());
						index_buffer_view.Format = DXGI_FORMAT_R16_UINT;

						render_primitive.index_buffer_view = index_buffer_view;
					}

					render_primitive.index_count = static_cast<UINT>(mesh.indices.size());
					render_primitive.instance_count = static_cast<UINT>(instance_count);
					
					scene_resources.primitives.emplace_back(std::move(render_primitive));
				}
			}			
		}

		{
			winrt::com_ptr<ID3D12Resource> constant_buffer = create_buffer(
				device,
				heap, heap_offset + D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
				sizeof(Pass_data),
				D3D12_RESOURCE_STATE_COPY_DEST
			);

			scene_resources.constant_buffers.emplace_back(
				std::move(constant_buffer)
			);

			scene_resources.camera = { {{ 0.0f, 0.0f, 0.0f }}, {}, static_cast<float>(EIGEN_PI) / 4.0f, 2.0f, { 0.1f, 21.0f } };
		}

		return scene_resources;
	}
}
