#include <array>
#include <filesystem>
#include <fstream>

#include <Eigen/Core>

#include <d3d12.h>

#include <winrt/base.h>

#include <nlohmann/json.hpp>

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


	template <class Value_type>
	void get_to_if_exists(nlohmann::json const& json, std::string_view key, std::optional<Value_type>& value)
	{
		nlohmann::json::const_iterator const location = json.find(key);

		if (location != json.end())
			value = location->get<Value_type>();
		else
			value = {};
	}


	struct Buffer
	{
		std::optional<std::string> uri;
		std::size_t byteLength;
	};

	void from_json(nlohmann::json const& json, Buffer& value)
	{
		get_to_if_exists(json, "uri", value.uri);
		json.at("byteLength").get_to(value.byteLength);
	}
	

	struct Scene
	{
		std::optional<std::string> name;
		std::optional<std::vector<std::size_t>> nodes;
	};

	void from_json(nlohmann::json const& json, Scene& value)
	{
		get_to_if_exists(json, "name", value.name);
		get_to_if_exists(json, "nodes", value.nodes);
	}


	struct Primitive
	{
		std::map<std::string, std::size_t> attributes;
		std::optional<std::size_t> indices_index;
		std::optional<std::size_t> material_index;
	};

	void from_json(nlohmann::json const& json, Primitive& value)
	{
		json.at("attributes").get_to(value.attributes);
		get_to_if_exists(json, "indices", value.indices_index);
		get_to_if_exists(json, "material", value.material_index);
	}


	struct Mesh
	{
		std::vector<Primitive> primitives;
		std::optional<std::string> name;
	};

	void from_json(nlohmann::json const& json, Mesh& value)
	{
		get_to_if_exists<std::string>(json, "name", value.name);
		json.at("primitives").get_to(value.primitives);
	}

	std::vector<std::byte> base64_decode(std::string_view input)
	{	
		constexpr std::array<std::uint8_t, 80> lookup_table
		{
			62,  255, 62,  255, 63,  52,  53, 54, 55, 56, 57, 58, 59, 60, 61, 255,
			255, 0,   255, 255, 255, 255, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
			10,  11,  12,  13,  14,  15,  16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
			255, 255, 255, 255, 63,  255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
			36,  37,  38,  39,  40,  41,  42, 43, 44, 45, 46, 47, 48, 49, 50, 51 
		};
		static_assert(sizeof(lookup_table) == 'z' - '+' + 1);

		std::vector<std::byte> output;
		output.reserve(input.size() * 3 / 4);

		{
			std::uint32_t bits{ 0 };
			std::uint8_t bit_count{ 0 };

			for (char c : input)
			{
				assert('+' <= c && c <= 'z');
				assert(lookup_table[c - '+'] < 64);
				
				c -= '+';

				bits = (bits << 6) + lookup_table[c];
				bit_count += 6;

				if (bit_count >= 8)
				{
					bit_count -= 8;
					output.push_back(static_cast<std::byte>((bits >> bit_count) & 0xFF));
				}
			}

			assert(bit_count == 0);
		}

		return output;
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
			nlohmann::json const gltf_json = []() -> nlohmann::json
			{
				std::filesystem::path filename{ "box.gltf" };
				std::ifstream file_stream{ filename };

				nlohmann::json json;
				file_stream >> json;
				return json;
			}();

			for (auto&[key, value] : gltf_json.items())
			{
				if (key == "buffers")
				{
					std::vector<Buffer> buffers;
					value.get_to(buffers);

					{
						Buffer const& buffer = buffers[0];

						std::vector<std::byte> buffer_data;
						buffer_data.reserve(buffer.byteLength);

						if (buffer.uri)
						{
							std::string const& uri = buffer.uri.value();
							
							char const* const prefix{ "data:application/octet-stream;base64," };
							std::size_t const prefix_size{ std::strlen(prefix) };
							assert(uri.compare(0, prefix_size, prefix) == 0 && "Uri format not supported");

							std::string_view const data_view{ uri.data() + prefix_size, uri.size() - prefix_size };
							assert(data_view.size() / 4 * 3 == buffer.byteLength && "Data content is ill-formed");

							buffer_data = base64_decode(data_view);
						}
					}
				}
				else if (key == "meshes")
				{
					std::vector<Mesh> meshes;
					value.get_to(meshes);
				}
			}
		}

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
