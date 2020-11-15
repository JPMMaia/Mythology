export module maia.ecs.component_chunk_group;

import maia.ecs.component;
import maia.ecs.entity;
import maia.ecs.shared_component;

import <array>;
import <cassert>;
import <cstddef>;
import <memory_resource>;
import <optional>;
import <span>;
import <vector>;
import <unordered_map>;

namespace Maia::ECS
{
    export template <class Component>
	struct Component_view
	{
		static_assert(!std::is_pointer_v<Component>);

		using Pointer_type = std::conditional_t<
			std::is_const_v<Component>,
			std::byte const*,
			std::byte*
		>;
		using Value_type = std::remove_const_t<Component>;

		Value_type get() const noexcept
		{
			Value_type component{};
			std::memcpy(&component, this->raw_data, sizeof(Component));
			return component;
		}

		void set(Value_type const& component) const noexcept
		{
			std::memcpy(this->raw_data, &component, sizeof(Component));
		}

		Pointer_type raw_data;
	};

	export template <class Component>
	struct Component_range_view
	{
		using Data_type = std::conditional_t<
			std::is_const_v<Component>,
			std::byte const,
			std::byte
		>;
		using Value_type = std::remove_const_t<Component>;

		Component_range_view() noexcept = default;

		Component_range_view(std::span<Data_type> const raw_data) noexcept :
			raw_data{raw_data}
		{
		}

		template <class Other_component>
		Component_range_view(Component_range_view<Other_component> const& other) noexcept :
			raw_data{other.raw_data}
		{
		}

		Value_type get(std::size_t const index) const noexcept
		{
			assert((index + 1) * sizeof(Value_type) <= this->raw_data.size_bytes());

			Value_type component{};
			std::memcpy(&component, this->raw_data.data() + index * sizeof(Value_type), sizeof(Value_type));
			return component;
		}

		void set(std::size_t const index, Value_type const& component) const noexcept
		{
			assert((index + 1) * sizeof(Value_type) <= this->raw_data.size_bytes());

			std::memcpy(this->raw_data.data() + index * sizeof(Value_type), &component, sizeof(Value_type));
		}

		std::size_t size() const
		{
			return this->raw_data.size() / sizeof(Value_type);
		}

		std::span<Data_type> raw_data;
	};

    using Component_chunk = std::pmr::vector<std::byte>;


	export struct Component_group_entity_index
	{
		std::size_t value;
	};

	export struct Component_group_entity_moved
	{
		Entity entity;
	};

	export struct Component_type_info_and_offset
	{
		Component_type_ID id;
		std::size_t offset;
		Component_size size;
	};

	export using Chunk_group_hash = std::size_t;
	using Chunk = std::pmr::vector<std::byte>;

	struct Chunk_group
	{
		std::pmr::vector<Chunk> chunks;
		std::size_t number_of_elements;
	};

	export class Component_chunk_group
	{
	public:

		using Index = std::size_t;

		Component_chunk_group(
			std::span<Component_type_info const> const component_type_infos,
			Shared_component_type_info const shared_component_type_info,
			std::size_t const number_of_entities_per_chunk,
			std::pmr::polymorphic_allocator<std::byte> const& chunk_allocator,
			std::pmr::polymorphic_allocator<std::byte> const& allocator
		) noexcept :
			m_chunk_groups{allocator},
			m_component_type_infos{allocator},
			m_number_of_entities_per_chunk{number_of_entities_per_chunk},
			m_chunk_allocator{chunk_allocator}
		{
			m_component_type_infos.reserve(component_type_infos.size());
			m_component_type_infos.assign(component_type_infos.begin(), component_type_infos.end());
			m_component_type_infos.push_back({get_component_type_id<Entity>(), sizeof(Entity)});
		}

		Index add_entity(Entity const entity, Chunk_group_hash const chunk_group_hash)
		{
			auto const location = m_chunk_groups.find(chunk_group_hash);

			if (location != m_chunk_groups.end())
			{
				Chunk_group& chunk_group = location->second;
				assert(!chunk_group.chunks.empty());

				std::size_t const chunk_group_capacity = m_number_of_entities_per_chunk * chunk_group.chunks.size();

				if (chunk_group.number_of_elements < chunk_group_capacity)
				{
					Chunk& chunk = chunk_group.chunks.back(); // TODO not the back one
				
					std::size_t const offset = 
						m_number_of_entities_per_chunk * get_component_offset(get_component_type_id<Entity>()) + 
						(chunk_group.number_of_elements % m_number_of_entities_per_chunk) * sizeof(Entity);

					assert((offset + sizeof(Entity)) <= chunk.size());
					std::memcpy(chunk.data() + offset, &entity, sizeof(Entity));
					
					++chunk_group.number_of_elements;

					return chunk_group.number_of_elements - 1;
				}
				else
				{
					Chunk new_chunk{m_chunk_allocator};
					new_chunk.resize(get_chunk_size());
					
					std::size_t const offset = 
						m_number_of_entities_per_chunk * get_component_offset(get_component_type_id<Entity>()) + 
						(0 % m_number_of_entities_per_chunk) * sizeof(Entity);

					assert((offset + sizeof(Entity)) <= new_chunk.size());
					std::memcpy(new_chunk.data() + offset, &entity, sizeof(Entity));

					++chunk_group.number_of_elements;

					chunk_group.chunks.push_back(std::move(new_chunk));

					return chunk_group.number_of_elements - 1;
				}
			}
			else
			{
				Chunk new_chunk{m_chunk_allocator};
				new_chunk.resize(get_chunk_size());
				
				std::size_t const offset = 
					m_number_of_entities_per_chunk * get_component_offset(get_component_type_id<Entity>()) + 
					(0 % m_number_of_entities_per_chunk) * sizeof(Entity);

				assert((offset + sizeof(Entity)) <= new_chunk.size());
				std::memcpy(new_chunk.data() + offset, &entity, sizeof(Entity));

				std::pmr::vector<Chunk> chunks{m_chunk_groups.get_allocator()};
				chunks.push_back(std::move(new_chunk));

				Chunk_group chunk_group{std::move(chunks), 1};

				m_chunk_groups.emplace(chunk_group_hash, std::move(chunk_group));

				return 0;
			}
		}

		std::optional<Component_group_entity_moved> remove_entity(Chunk_group_hash const chunk_group_hash, Index const index) noexcept
		{
			Chunk_group& chunk_group = m_chunk_groups.at(chunk_group_hash);

			if ((index + 1) == chunk_group.number_of_elements)
			{
				--chunk_group.number_of_elements;

				return std::nullopt;
			}
			else
			{
				std::size_t const entity_to_remove_index = index;
				std::size_t const entity_to_move_index = chunk_group.number_of_elements - 1;

				std::size_t const entity_to_remove_chunk_index = entity_to_remove_index / m_number_of_entities_per_chunk;
				Chunk& entity_to_remove_chunk = chunk_group.chunks[entity_to_remove_chunk_index];

				std::size_t const entity_to_move_chunk_index = entity_to_move_index / m_number_of_entities_per_chunk;
				Chunk const& entity_to_move_chunk = chunk_group.chunks[entity_to_move_chunk_index];
								
				Entity const entity_to_move = get_entity(chunk_group_hash, entity_to_move_index);

				std::size_t total_component_size = 0;

				for (Component_type_info const& type_info : m_component_type_infos)
				{
					std::size_t const source_offset = 
						m_number_of_entities_per_chunk * total_component_size + 
						(entity_to_move_index % m_number_of_entities_per_chunk) * type_info.size;

					std::size_t const destination_offset = 
						m_number_of_entities_per_chunk * total_component_size + 
						(entity_to_remove_index % m_number_of_entities_per_chunk) * type_info.size;

					assert((source_offset + type_info.size) <= entity_to_move_chunk.size());
					assert((destination_offset + type_info.size) <= entity_to_remove_chunk.size());
					std::memcpy(entity_to_remove_chunk.data() + destination_offset, entity_to_move_chunk.data() + source_offset, type_info.size);

					total_component_size += type_info.size;
				}

				--chunk_group.number_of_elements;

				return Component_group_entity_moved{entity_to_move};
			}
		}

		Entity get_entity(Chunk_group_hash const chunk_group_hash, Index const index) const noexcept
		{
			return get_component_value<Entity>(chunk_group_hash, index);
		}

		template <Concept::Component Component_t>
		Component_t get_component_value(Chunk_group_hash const chunk_group_hash, Index const index) const noexcept
		{
			Chunk_group const& chunk_group = m_chunk_groups.at(chunk_group_hash);

			std::size_t const chunk_index = index / m_number_of_entities_per_chunk;
			Chunk const& chunk = chunk_group.chunks[chunk_index];

			Component_type_ID const component_type_id = get_component_type_id<Component_t>();
			std::size_t const offset = 
				 m_number_of_entities_per_chunk * get_component_offset(component_type_id) + 
				(index % m_number_of_entities_per_chunk) * sizeof(Component_t);

			Component_t value{};

			assert((offset + sizeof(Component_t)) <= chunk.size());
			std::memcpy(&value, chunk.data() + offset, sizeof(Component_t));

			return value;
		}

		template <Concept::Component Component_t>
		void set_component_value(Chunk_group_hash const chunk_group_hash, Index const index, Component_t const& value) noexcept
		{
			Chunk_group& chunk_group = m_chunk_groups.at(chunk_group_hash);

			std::size_t const chunk_index = index / m_number_of_entities_per_chunk;
			Chunk& chunk = chunk_group.chunks[chunk_index];

			Component_type_ID const component_type_id = get_component_type_id<Component_t>();
			std::size_t const offset = 
				m_number_of_entities_per_chunk * get_component_offset(component_type_id) + 
				(index % m_number_of_entities_per_chunk) * sizeof(Component_t);

			assert((offset + sizeof(Component_t)) <= chunk.size());
			std::memcpy(chunk.data() + offset, &value, sizeof(Component_t));
		}

		void shrink_to_fit(Chunk_group_hash const chunk_group_hash) noexcept
		{
		}

		bool has_component_type(Component_type_ID const id) const noexcept
		{
			auto const is_component_type = [&id](Component_type_info const info) -> bool
			{
				return info.id == id;
			};

			auto const location = std::find_if(
				m_component_type_infos.begin(),
				m_component_type_infos.end(),
				is_component_type
			);

			return location != m_component_type_infos.end();
		}

		bool has_shared_component_type(Shared_component_type_ID const id) const noexcept
		{
			return {};
		}

		std::size_t number_of_entities() const noexcept
		{
			return {};
		}

		std::size_t number_of_entities(Chunk_group_hash const chunk_group_hash) const noexcept
		{
			std::size_t count = 0;

			for (std::pair<Chunk_group_hash, Chunk_group> const& chunk_group : m_chunk_groups)
			{
				count += chunk_group.second.number_of_elements;
			}

			return count;
		}

		std::size_t number_of_chunks() const noexcept
		{
			std::size_t count = 0;

			for (std::pair<Chunk_group_hash, Chunk_group> const& chunk_group : m_chunk_groups)
			{
				count += chunk_group.second.chunks.size();
			}

			return count;
		}

		std::size_t number_of_chunks(Chunk_group_hash const chunk_group_hash) const noexcept
		{
			auto const location = m_chunk_groups.find(chunk_group_hash);

			if (location != m_chunk_groups.end())
			{
				Chunk_group const& chunk_group = m_chunk_groups.at(chunk_group_hash);

				return chunk_group.chunks.size();
			}
			else
			{
				return 0;
			}
		}

	private:

		std::size_t get_chunk_size() const noexcept
		{
			std::size_t const total_component_size = [this]
			{
				std::size_t total_component_size = 0;

				for (Component_type_info const& type_info : m_component_type_infos)
				{
					total_component_size += type_info.size;
				}

				return total_component_size;
			}();

			return m_number_of_entities_per_chunk * total_component_size;
		}

		std::size_t get_component_offset(Component_type_ID const component_type_id) const noexcept
		{
			auto const is_type_info = [&component_type_id](Component_type_info const type_info) -> bool
			{
				return type_info.id == component_type_id;
			};

			auto const location = std::find_if(
				m_component_type_infos.begin(),
				m_component_type_infos.end(),
				is_type_info
			);

			std::ptrdiff_t const target_type_info_index = std::distance(m_component_type_infos.begin(), location);

			std::size_t offset = 0;

			for (std::size_t type_info_index = 0; type_info_index < target_type_info_index; ++type_info_index)
			{
				Component_type_info const& type_info = m_component_type_infos[type_info_index];
				offset += type_info.size;
			}

			return offset;
		}

		std::size_t get_entity_component_chunk_offset() const noexcept
		{
			return get_component_offset(get_component_type_id<Entity>());
		}

	private:

		std::pmr::unordered_map<Chunk_group_hash, Chunk_group> m_chunk_groups;
		std::pmr::vector<Component_type_info> m_component_type_infos;
		std::size_t m_number_of_entities_per_chunk;
		std::pmr::polymorphic_allocator<std::byte> m_chunk_allocator;

	};
}