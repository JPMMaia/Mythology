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

	using Shared_component_hash = std::size_t;
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
			m_component_type_infos{component_type_infos.begin(), component_type_infos.end(), allocator},
			m_number_of_entities_per_chunk{number_of_entities_per_chunk},
			m_chunk_allocator{chunk_allocator}
		{
		}

		Index add_entity(Entity const entity)
		{
			constexpr Shared_component_hash empty_shared_component_hash = 0;

			auto const location = m_chunk_groups.find(empty_shared_component_hash);

			if (location != m_chunk_groups.end())
			{
				Chunk_group& chunk_group = location->second;
				assert(!chunk_group.chunks.empty());

				std::size_t const chunk_group_capacity = m_number_of_entities_per_chunk * chunk_group.chunks.size();

				if (chunk_group.number_of_elements < chunk_group_capacity)
				{
					Chunk& chunk = chunk_group.chunks.back(); // TODO not the back one
				
					// TODO Add new entity to chunk
					++chunk_group.number_of_elements;

					return chunk_group.number_of_elements - 1;
				}
				else
				{
					Chunk new_chunk{m_chunk_allocator};
					new_chunk.resize(get_chunk_size());
					
					// TODO Add new entity to chunk
					++chunk_group.number_of_elements;

					chunk_group.chunks.push_back(std::move(new_chunk));

					return chunk_group.number_of_elements - 1;
				}
			}
			else
			{
				Chunk new_chunk{m_chunk_allocator};
				new_chunk.resize(get_chunk_size());
				// TODO Add new entity to chunk

				std::pmr::vector<Chunk> chunks{m_chunk_groups.get_allocator()};
				chunks.push_back(std::move(new_chunk));

				Chunk_group chunk_group{std::move(chunks), 1};

				m_chunk_groups.emplace(empty_shared_component_hash, std::move(chunk_group));

				return 0;
			}
		}

		template <Concept::Shared_component Shared_component_t>
		Index add_entity(Entity const entity, Shared_component_t const& shared_component)
		{
			return {};
		}

		Component_group_entity_moved remove_entity(Index const index) noexcept
		{
			return {};
		}

		Entity get_entity(Index const index) const noexcept
		{
			return {};
		}

		template <Concept::Component Component_t>
		Component_t get_component_value(Index const index) const noexcept
		{
			return {};
		}

		template <Concept::Component Component_t>
		void set_component_value(Index const index, Component_t const& value) noexcept
		{
		}

		void shrink_to_fit() noexcept
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

		std::size_t number_of_chunks() const noexcept
		{
			std::size_t count = 0;

			for (std::pair<Shared_component_hash, Chunk_group> const& chunk_group : m_chunk_groups)
			{
				count += chunk_group.second.chunks.size();
			}

			return count;
		}

		template <Concept::Shared_component Shared_component_t>
		std::size_t number_of_chunks(Shared_component_t const& shared_component) const noexcept
		{
			return {};
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

	private:

		std::pmr::unordered_map<Shared_component_hash, Chunk_group> m_chunk_groups;
		std::pmr::vector<Component_type_info> m_component_type_infos;
		std::size_t m_number_of_entities_per_chunk;
		std::pmr::polymorphic_allocator<std::byte> m_chunk_allocator;

	};
}