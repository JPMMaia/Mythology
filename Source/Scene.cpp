#include <array>

#include <Eigen/Core>

#include <d3d12.h>

#include <winrt/base.h>

#include <Maia/Renderer/D3D12/Utilities/D3D12_utilities.hpp>

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
				{ { -1.0f, -1.0f, 0.5f, 1.0f } },
				{ { 1.0f, -1.0f, 0.5f, 1.0f } },
				{ { 0.0f, 1.0f, 0.5f, 1.0f } },
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
			gsl::span<Instance_data> instances_data)
		{
			using namespace Maia::Renderer::D3D12;

			upload_buffer_data(
				command_list,
				*buffer.value, base_buffer_offset,
				upload_buffer, upload_buffer_offset,
				instances_data
			);
		}

		std::vector<Instance_data> create_instances_data()
		{
			Eigen::Matrix4f world_matrix;
			world_matrix <<
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f;
			return { { world_matrix } };
		}
	}

	Maia::Mythology::D3D12::Scene_resources load(Maia::GameEngine::Entity_manager& entity_manager, Maia::Mythology::D3D12::Render_resources const& render_resources)
	{
		ID3D12Device& device = *render_resources.device;
		ID3D12Heap& heap = *render_resources.buffers_heap;
		UINT64 heap_offset = render_resources.buffers_heap_offset;
		ID3D12Resource& upload_buffer = *render_resources.upload_buffer;
		UINT64 const upload_buffer_offset = render_resources.upload_buffer_offset;
		ID3D12GraphicsCommandList& command_list = *render_resources.command_list;

		using namespace Maia::Renderer::D3D12;

		Maia::Mythology::D3D12::Scene_resources scene_resources;

		{
			Colored_mesh const mesh = create_triangle_mesh();
			std::vector<Instance_data> instances = create_instances_data();

			UINT64 positions_size_bytes = mesh.vertices.positions.size() * sizeof(decltype(mesh.vertices.positions)::value_type);
			UINT64 color_size_bytes = mesh.vertices.colors.size() * sizeof(decltype(mesh.vertices.colors)::value_type);
			UINT64 indices_size_bytes = mesh.indices.size() * sizeof(decltype(mesh.indices)::value_type);
			UINT64 const geometry_buffer_width =
				positions_size_bytes + color_size_bytes + indices_size_bytes;
			UINT64 const instances_data_size_bytes = sizeof(Instance_data) * instances.size();
			UINT64 const total_buffer_width = geometry_buffer_width + instances_data_size_bytes;

			D3D12::Geometry_and_instances_buffer geometry_and_instances_buffer = 
			{
				create_buffer(
					device,
					heap, heap_offset,
					total_buffer_width,
					D3D12_RESOURCE_STATE_COPY_DEST
				)
			};

			{
				upload_geometry_data(mesh, geometry_and_instances_buffer, 0, upload_buffer, upload_buffer_offset, command_list);
				upload_instances_data(geometry_and_instances_buffer, geometry_buffer_width, upload_buffer, upload_buffer_offset + geometry_buffer_width, command_list, instances);
			}

			{
				Maia::Mythology::D3D12::Render_primitive render_primitive{};

				{
					D3D12_VERTEX_BUFFER_VIEW const position_buffer_view = [&]() -> D3D12_VERTEX_BUFFER_VIEW
					{
						D3D12_VERTEX_BUFFER_VIEW buffer_view;
						buffer_view.BufferLocation = geometry_and_instances_buffer.value->GetGPUVirtualAddress();

						using data_type = decltype(mesh.vertices.positions)::value_type;
						gsl::span<data_type const> const data_view = mesh.vertices.positions;
						buffer_view.SizeInBytes = static_cast<UINT>(data_view.size_bytes());
						buffer_view.StrideInBytes = sizeof(data_type);

						return buffer_view;
					}();

					D3D12_VERTEX_BUFFER_VIEW const color_buffer_view = [&]() -> D3D12_VERTEX_BUFFER_VIEW
					{
						D3D12_VERTEX_BUFFER_VIEW buffer_view;
						buffer_view.BufferLocation = geometry_and_instances_buffer.value->GetGPUVirtualAddress() +
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
						buffer_view.BufferLocation = geometry_and_instances_buffer.value->GetGPUVirtualAddress() + 
							geometry_buffer_width;

						using data_type = Instance_data;
						gsl::span<data_type const> const data_view = instances;
						buffer_view.SizeInBytes = static_cast<UINT>(data_view.size_bytes());
						buffer_view.StrideInBytes = sizeof(data_type);

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
					index_buffer_view.BufferLocation = geometry_and_instances_buffer.value->GetGPUVirtualAddress() +
						positions_size_bytes + color_size_bytes;
					
					using data_type = decltype(mesh.indices)::value_type;
					gsl::span<data_type const> const data_view = mesh.indices;
					index_buffer_view.SizeInBytes = static_cast<UINT>(data_view.size_bytes());
					index_buffer_view.Format = DXGI_FORMAT_R16_UINT;

					render_primitive.index_buffer_view = index_buffer_view;
				}

				render_primitive.instances_location = {};
				render_primitive.index_count = 3;
				render_primitive.instance_count = 1;
				scene_resources.primitives.emplace_back(std::move(render_primitive));
			}

			scene_resources.geometry_and_instances_buffers.emplace_back(
				std::move(geometry_and_instances_buffer)
			);
		}

		return scene_resources;
	}
}
