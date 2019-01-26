#include <fstream>
#include <vector>

#include <Maia/GameEngine/Entity_manager.hpp>
#include <Maia/GameEngine/Systems/Transform_system.hpp>

#include <Maia/Renderer/D3D12/Utilities/D3D12_utilities.hpp>
#include <Maia/Utilities/glTF/gltf.hpp>

#include "Load_scene_system.hpp"

using namespace Maia::Renderer::D3D12;

namespace Maia::Mythology::D3D12
{
	Load_scene_system::Load_scene_system(ID3D12Device& device) :
		m_device{ device },
		m_command_queue{ create_command_queue(device, D3D12_COMMAND_LIST_TYPE_COPY, 0, D3D12_COMMAND_QUEUE_FLAG_NONE, 0) },
		m_command_allocator{ create_command_allocator(device, D3D12_COMMAND_LIST_TYPE_COPY) },
		m_command_list{ create_closed_graphics_command_list(device, 0, D3D12_COMMAND_LIST_TYPE_COPY, *m_command_allocator) },
		m_upload_heap{ create_upload_heap(device, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT) },
		m_upload_buffer{ create_buffer(device, *m_upload_heap, 0, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, D3D12_RESOURCE_STATE_COPY_SOURCE) }
	{
	}

	namespace
	{
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

		std::vector<std::byte> generate_byte_data(std::string_view const uri, std::size_t const byte_length)
		{
			char const* const prefix{ "data:application/octet-stream;base64," };
			std::size_t const prefix_size{ std::strlen(prefix) };
			assert(uri.compare(0, prefix_size, prefix) == 0 && "Uri format not supported");

			std::string_view const data_view{ uri.data() + prefix_size, uri.size() - prefix_size };
			assert(data_view.size() / 4 * 3 == byte_length && "Data content is ill-formed");

			return base64_decode(data_view);
		}
	}

	Scenes_resources Load_scene_system::load(std::filesystem::path const& gltf_file_path)
	{
		using namespace Maia::Utilities::glTF;

		Gltf const gltf = [&]() -> Gltf
		{
			nlohmann::json const gltf_json = [&]() -> nlohmann::json
			{
				std::ifstream file_stream{ gltf_file_path };

				nlohmann::json json;
				file_stream >> json;
				return json;
			}();

			return gltf_json.get<Gltf>();
		}();

		Geometry_resources geometry_resources = [&]() -> Geometry_resources
		{
			if (gltf.buffers)
			{
				using namespace Maia::Renderer::D3D12;

				UINT64 const geometry_size_bytes = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

				winrt::com_ptr<ID3D12Heap> geometry_heap
				{
					create_buffer_heap(m_device, geometry_size_bytes)
				};

				Geometry_buffer geometry_buffer
				{
					create_buffer(
						m_device,
						*geometry_heap, 0,
						geometry_size_bytes,
						D3D12_RESOURCE_STATE_COPY_DEST
					)
				};

				std::vector<UINT64> geometry_offsets;
				geometry_offsets.reserve(gltf.buffers->size());


				UINT64 current_geometry_offset_bytes = 0;

				for (Buffer const& buffer : *gltf.buffers)
				{
					if (buffer.uri)
					{
						assert(current_geometry_offset_bytes + buffer.byte_length <= D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);

						std::vector<std::byte> const buffer_data =
							generate_byte_data(*buffer.uri, buffer.byte_length);

						upload_buffer_data<std::byte>(
							*m_command_list,
							*geometry_buffer.value, current_geometry_offset_bytes,
							*m_upload_buffer, current_geometry_offset_bytes,
							buffer_data
							);

						geometry_offsets.push_back(current_geometry_offset_bytes);

						current_geometry_offset_bytes += buffer_data.size();
					}
				}

				return { std::move(geometry_heap), std::move(geometry_buffer), std::move(geometry_offsets) };
			}
			else
			{
				return {};
			}
		}();

		if (gltf.materials)
		{
			for (Material const& material : *gltf.materials)
			{
				// TODO
			}
		}

		std::vector<Mesh_view> mesh_views = [&]() -> std::vector<Mesh_view>
		{
			std::vector<Mesh_view> mesh_views;

			if (gltf.meshes)
			{
				mesh_views.reserve(gltf.meshes->size());

				{
					gsl::span<Accessor const> accessors{ *gltf.accessors };
					gsl::span<Buffer_view const> buffer_views{ *gltf.buffer_views };

					for (Mesh const& mesh : *gltf.meshes)
					{
						std::vector<D3D12::Submesh_view> submesh_views;
						submesh_views.reserve(mesh.primitives.size());

						for (Primitive const& primitive : mesh.primitives)
						{
							D3D12::Submesh_view submesh_view{};

							submesh_view.vertex_buffer_views.reserve(primitive.attributes.size());
							for (std::pair<const std::string, size_t> const& attribute : primitive.attributes)
							{
								Accessor const& accessor = accessors[attribute.second];

								if (accessor.buffer_view_index)
								{
									Buffer_view const& buffer_view = buffer_views[*accessor.buffer_view_index];

									D3D12_GPU_VIRTUAL_ADDRESS const base_buffer_address =
										geometry_resources.buffer.value->GetGPUVirtualAddress() +
										geometry_resources.offsets[buffer_view.buffer_index];

									D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;
									vertex_buffer_view.BufferLocation =
										base_buffer_address + buffer_view.byte_offset;
									vertex_buffer_view.SizeInBytes = static_cast<UINT>(buffer_view.byte_length);
									vertex_buffer_view.StrideInBytes = size_of(accessor.component_type) * size_of(accessor.type);
									submesh_view.vertex_buffer_views.push_back(vertex_buffer_view);
								}
							}

							if (primitive.indices_index)
							{
								Accessor const& accessor = accessors[*primitive.indices_index];
								assert(accessor.component_type == Component_type::Unsigned_short || accessor.component_type == Component_type::Unsigned_int);

								if (accessor.buffer_view_index)
								{
									Buffer_view const& buffer_view = buffer_views[*accessor.buffer_view_index];

									D3D12_GPU_VIRTUAL_ADDRESS const base_buffer_address =
										geometry_resources.buffer.value->GetGPUVirtualAddress() +
										geometry_resources.offsets[buffer_view.buffer_index];

									D3D12_INDEX_BUFFER_VIEW	index_buffer_view;
									index_buffer_view.BufferLocation =
										base_buffer_address + buffer_view.byte_offset;
									index_buffer_view.SizeInBytes = static_cast<UINT>(buffer_view.byte_length);
									index_buffer_view.Format = accessor.component_type == Component_type::Unsigned_int ?
										DXGI_FORMAT_R32_UINT :
										DXGI_FORMAT_R16_UINT;
									submesh_view.index_buffer_view = index_buffer_view;
									submesh_view.index_count = static_cast<UINT>(accessor.count);
								}
							}

							submesh_views.push_back(submesh_view);
						}

						mesh_views.push_back({ std::move(submesh_views) });
					}
				}
			}

			return mesh_views;
		}();

		std::vector<Maia::Utilities::glTF::Scene> scenes = [&]() -> std::vector<Maia::Utilities::glTF::Scene>
		{
			if (gltf.scenes)
			{
				return *gltf.scenes;
			}
			else
			{
				return {};
			}
		}();

		std::size_t scene_index = gltf.scene_index ? *gltf.scene_index : 0;

		return { std::move(geometry_resources), std::move(mesh_views), std::move(scenes), scene_index };
	}

	void create_entities(
		Maia::Utilities::glTF::Scene const& scene,
		gsl::span<Maia::Utilities::glTF::Node const> nodes,
		std::size_t const mesh_count,
		Maia::GameEngine::Entity_manager& entity_manager
	)
	{
		using namespace Maia::GameEngine;
		using namespace Maia::GameEngine::Systems;
		using namespace Maia::Utilities::glTF;

		using Static_entity_type = Entity_type<Transform_matrix>;

		std::vector<Static_entity_type> entity_types = [&]() -> std::vector<Static_entity_type>
		{
			std::vector<Static_entity_type> entity_types;
			entity_types.reserve(mesh_count);

			for (size_t i = 0; i < entity_types.capacity(); ++i)
			{
				entity_types.push_back(entity_manager.create_entity_type<Transform_matrix>(10));
			}

			if (scene.nodes)
			{
				for (std::size_t const node_index : *scene.nodes)
				{
					Node const& node = nodes[node_index];

					if (node.mesh_index)
					{
						Transform_matrix const transform_matrix = 
							create_transform({ node.translation }, { node.rotation });

						entity_manager.create_entity(
							entity_types[*node.mesh_index],
							transform_matrix
						);
					}
				}
			}

			return entity_types;
		}();

		// TODO return entity_types
	}

	using namespace Maia::GameEngine;
	using namespace Maia::GameEngine::Systems;

	struct Scene_instance_data
	{
		winrt::com_ptr<ID3D12Heap> instance_heap;
		Instance_buffer instance_buffer;
	};

	Scene_instance_data create_instance_buffers(
		ID3D12Device& device, 
		Maia::GameEngine::Entity_manager const& entity_manager, 
		gsl::span<Entity_type<Transform_matrix> const> entity_types
	)
	{
		std::size_t const size_in_bytes = [&]() -> std::size_t
		{
			std::size_t count{ 0 };

			for (Entity_type<Transform_matrix> const entity_type : entity_types)
			{
				Component_group const& component_group = entity_manager.get_component_group(entity_type.id);

				count += component_group.size();
			}

			return count * sizeof(Transform_matrix);
		}();

		winrt::com_ptr<ID3D12Heap> instance_heap = 
			create_buffer_heap(device, size_in_bytes);

		Instance_buffer instance_buffer
		{
			create_buffer(
				device, 
				*instance_heap, 0,
				size_in_bytes,
				D3D12_RESOURCE_STATE_COPY_DEST
			)
		};

		return { std::move(instance_heap), std::move(instance_buffer) };
	}

	UINT64 upload_instance_data(
		ID3D12Device& device,
		ID3D12GraphicsCommandList& command_list,
		Maia::GameEngine::Entity_manager const& entity_manager,
		gsl::span<Entity_type<Transform_matrix> const> entity_types,
		Instance_buffer const& instance_buffer,
		ID3D12Resource& upload_buffer, UINT64 const upload_buffer_offset_in_bytes
	)
	{
		UINT64 instance_buffer_offset_in_bytes{ 0 };

		for (Entity_type<Transform_matrix> const entity_type : entity_types)
		{
			Component_group const& component_group = entity_manager.get_component_group(entity_type.id);

			for (std::size_t chunk_index = 0; chunk_index < component_group.num_chunks(); ++chunk_index)
			{
				gsl::span<Transform_matrix const> const transform_matrices =
					component_group.components<Transform_matrix>(chunk_index);

				upload_buffer_data(
					command_list,
					*instance_buffer.value, instance_buffer_offset_in_bytes,
					upload_buffer, upload_buffer_offset_in_bytes,
					transform_matrices
				);

				instance_buffer_offset_in_bytes += transform_matrices.size_bytes();
			}
		}

		return instance_buffer_offset_in_bytes;
	}
}
