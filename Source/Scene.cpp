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
			D3D12::Geometry_and_instances_buffer& buffer,
			ID3D12Device& device,
			ID3D12Resource& upload_buffer, UINT64& upload_buffer_offset, // TODO create a struct for both
			ID3D12GraphicsCommandList& command_list
		)
		{
			using namespace Maia::Renderer::D3D12;

			Mapped_memory mapped_memory{ *buffer.value, 0, {} };
			UINT64 offset = 0;

			{
				gsl::span<Position const> const positions{ mesh.vertices.positions };
				upload_buffer_data(
					mapped_memory,
					command_list,
					*buffer.value, offset,
					upload_buffer, upload_buffer_offset + offset,
					positions
				);
				offset += positions.size_bytes();
			}

			{
				gsl::span<Color const> const colors{ mesh.vertices.colors };
				upload_buffer_data(
					mapped_memory,
					command_list,
					*buffer.value, offset,
					upload_buffer, upload_buffer_offset + offset,
					colors
				);
				offset += colors.size_bytes();
			}

			{
				gsl::span<decltype(mesh.indices)::value_type> const indices{ mesh.indices };
				upload_buffer_data(
					mapped_memory,
					command_list,
					*buffer.value, offset,
					upload_buffer, upload_buffer_offset + offset,
					indices
				);
				offset += indices.size_bytes();
			}
		}

		void upload_instances_data(D3D12::Geometry_and_instances_buffer& buffer)
		{
		}
	}

	void load(Maia::GameEngine::Entity_manager& entity_manager, Maia::Mythology::D3D12::Render_resources& render_resources)
	{
		ID3D12Device& device;
		ID3D12Heap& heap;
		UINT64 heap_offset = 0;
		ID3D12Resource& upload_buffer;
		UINT64 upload_buffer_offset;
		ID3D12GraphicsCommandList& command_list;

		using namespace Maia::Renderer::D3D12;

		{
			Colored_mesh const mesh = create_triangle_mesh();

			UINT64 const geometry_buffer_width =
				mesh.vertices.positions.size() * sizeof(decltype(mesh.vertices.positions)::value_type) +
				mesh.vertices.colors.size() * sizeof(decltype(mesh.vertices.colors)::value_type) +
				mesh.indices.size() * sizeof(decltype(mesh.indices)::value_type);
			UINT64 const instances_buffer_width = 0;
			UINT64 const total_buffer_width = geometry_buffer_width + instances_buffer_width;

			D3D12::Geometry_and_instances_buffer geometry_and_instances_buffer = 
			{
				create_buffer(
					device,
					heap, heap_offset,
					total_buffer_width,
					D3D12_RESOURCE_STATE_COPY_DEST
				)
			};

			upload_geometry_data(mesh, geometry_and_instances_buffer, device, upload_buffer, upload_buffer_offset, command_list);

			heap_offset += total_buffer_width;
		}
	}
}
