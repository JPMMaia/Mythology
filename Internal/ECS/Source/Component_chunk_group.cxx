export module maia.ecs.component_chunk_group;

import maia.ecs.component;
import maia.ecs.entity;
import maia.ecs.shared_component;

import <array>;
import <cassert>;
import <cstddef>;
import <iterator>;
import <memory_resource>;
import <optional>;
import <ostream>;
import <span>;
import <vector>;
import <unordered_map>;

namespace Maia::ECS
{
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

	std::size_t get_number_of_elements(
		std::size_t const number_of_elements_in_chunk_group,
		std::size_t const number_of_elements_per_chunk,
		std::size_t const chunk_index
	) noexcept
	{
		std::size_t const capacity_at_previous_chunk = chunk_index * number_of_elements_per_chunk;
		std::size_t const capacity_at_current_chunk = capacity_at_previous_chunk + number_of_elements_per_chunk;

		if (number_of_elements_in_chunk_group >= capacity_at_current_chunk)
		{
			return number_of_elements_per_chunk;
		}
		else if (number_of_elements_in_chunk_group > capacity_at_previous_chunk)
		{
			return number_of_elements_in_chunk_group - capacity_at_previous_chunk;
		}
		else
		{
			return 0;
		}
	}

	template <typename Component_t>
	struct Component_view
	{
		operator Component_t() const noexcept
		{
			Component_t component;
			std::memcpy(&component, this->pointer, sizeof(Component_t));
			return component;
		}

		Component_view& operator=(Component_t const& component) noexcept
		{
			std::memcpy(this->pointer, &component, sizeof(Component_t));
			
			return *this;
		}

		Component_view& operator=(Component_t&& component) noexcept
		{
			std::memcpy(this->pointer, &component, sizeof(Component_t));

			return *this;
		}

		std::byte* pointer;
	};

	export template <typename T>
	std::ostream& operator<<(std::ostream& output_stream, Component_view<T> const view) noexcept
	{
		T const value = view;
		output_stream << value;
		
		return output_stream;
	}

	export template <typename T>
	class Component_iterator
	{
	public:

		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using pointer = Component_view<T>*;
		using reference = Component_view<T>;
		using iterator_category = std::random_access_iterator_tag;

		explicit Component_iterator(std::byte* const data_pointer = nullptr) noexcept :
			m_view{data_pointer}
		{
		}

		reference operator*() const noexcept
		{
			return m_view;
		}

		bool operator==(Component_iterator const rhs) const noexcept
		{
			return m_view.pointer == rhs.m_view.pointer;
		}

		Component_iterator& operator++() noexcept
		{
			m_view.pointer += sizeof(T);
			return *this;
		}

		Component_iterator operator++(int) noexcept
		{
			Component_iterator temp{m_view.pointer};
			m_view.pointer += sizeof(T);
			return temp;
		}

		Component_iterator& operator--() noexcept
		{
			m_view.pointer -= sizeof(T);
			return *this;
		}

		Component_iterator operator--(int) noexcept
		{
			Component_iterator temp{m_view.pointer};
			m_view.pointer -= sizeof(T);
			return temp;
		}

		bool operator<(Component_iterator const rhs) const noexcept
		{
			return m_view.pointer < rhs.m_view.pointer;
		}

		bool operator>(Component_iterator const rhs) const noexcept
		{
			return m_view.pointer > rhs.m_view.pointer;
		}

		bool operator<=(Component_iterator const rhs) const noexcept
		{
			return m_view.pointer <= rhs.m_view.pointer;
		}

		bool operator>=(Component_iterator const rhs) const noexcept
		{
			return m_view.pointer <= rhs.m_view.pointer;
		}

		difference_type operator-(Component_iterator const rhs) const noexcept
		{
			assert((m_view.pointer - rhs.m_view.pointer) % sizeof(T) == 0);

			return (m_view.pointer - rhs.m_view.pointer) / sizeof(T);
		}

		Component_iterator& operator+=(difference_type const n) noexcept
		{
			m_view.pointer += n * sizeof(T);
			return *this;
		}

		Component_iterator operator+(difference_type const n) const noexcept
		{
			return Component_iterator{m_view.pointer + n * sizeof(T)};
		}

		Component_iterator& operator-=(difference_type const n) noexcept
		{
			m_view.pointer -= n * sizeof(T);
			return *this;
		}

		Component_iterator operator-(difference_type const n) const noexcept
		{
			return Component_iterator{m_view.pointer - n * sizeof(T)};
		}

		reference operator[](difference_type const index) const noexcept
		{
			return *(m_view.pointer + index * sizeof(T));
		}

	private:

		Component_view<T> m_view = nullptr;

	};

	export template <typename T>
	Component_iterator<T> operator+(typename Component_iterator<T>::difference_type const lhs, Component_iterator<T> const rhs) noexcept
	{
		return rhs + lhs;
	}

	export template <typename T>
	class Component_chunk_view
	{
	public:

		using Iterator = Component_iterator<T>;

		Component_chunk_view() noexcept = default;

		Component_chunk_view(Iterator const begin, Iterator const end) noexcept :
			m_begin{begin},
			m_end{end}
		{
		}

		Iterator begin() const noexcept
		{
			return m_begin;
		}

		Iterator end() const noexcept
		{
			return m_end;
		}

	private:

		Iterator m_begin{};
		Iterator m_end{};
	};

	export template <typename T>
	class Component_chunk_iterator
	{
	public:

		using difference_type = std::ptrdiff_t;
		using value_type = Component_chunk_view<T>;
		using pointer = Component_chunk_view<T>*;
		using reference = Component_chunk_view<T>&;
		using iterator_category = std::bidirectional_iterator_tag;

		Component_chunk_iterator() noexcept = default;

		Component_chunk_iterator(
			Chunk_group& chunk_group,
			std::size_t const chunk_index,
			std::size_t const number_of_elements_per_chunk,
			std::size_t const component_offset
		) noexcept :
			m_chunk_group{chunk_group},
			m_current_chunk_index{chunk_index},
			m_number_of_elements_per_chunk{number_of_elements_per_chunk},
			m_component_offset{component_offset},
			m_component_chunk_view{create_component_view()}
		{
		}

		reference operator*() const noexcept
		{
			return m_component_chunk_view;
		}

		pointer operator->() const noexcept
		{
			return &m_component_chunk_view;
		}

		bool operator==(Component_chunk_iterator const rhs) const noexcept
		{
			return &m_chunk_group == &rhs.m_chunk_group
				&& m_current_chunk_index == rhs.m_current_chunk_index;
		}

		Component_chunk_iterator& operator++() noexcept
		{
			++m_current_chunk_index;
			m_component_chunk_view = create_component_view();

			return *this;
		}

		Component_chunk_iterator operator++(int) noexcept
		{
			Component_chunk_iterator const temp
			{
				m_chunk_group,
				m_current_chunk_index,
				m_number_of_elements_per_chunk,
				m_component_offset
			};

			++(*this);
			
			return temp;
		}

		Component_chunk_iterator& operator--() noexcept
		{
			--m_current_chunk_index;
			m_component_chunk_view = create_component_view();
			return *this;
		}

		Component_chunk_iterator operator--(int) noexcept
		{
			Component_chunk_iterator const temp
			{
				m_chunk_group,
				m_current_chunk_index,
				m_number_of_elements_per_chunk,
				m_component_offset
			};

			--(*this);
			
			return temp;
		}

		difference_type operator-(Component_chunk_iterator const rhs) const noexcept
		{
			assert(m_chunk_group == rhs.m_chunk_group);

			if (m_current_chunk_index >= rhs.m_current_chunk_index) [[likely]]
			{
				return m_current_chunk_index - rhs.m_current_chunk_index;
			}
			else
			{
				return -static_cast<difference_type>(rhs.m_current_chunk_index - m_current_chunk_index);
			}
		}

	private:

		Component_chunk_view<T> create_component_view() const noexcept
		{
			std::size_t const number_of_elements_in_chunk = get_number_of_elements(
				m_chunk_group.number_of_elements,
				m_number_of_elements_per_chunk,
				m_current_chunk_index
			);

			if (number_of_elements_in_chunk != 0) [[likely]]
			{
				Chunk& chunk = m_chunk_group.chunks[m_current_chunk_index];

				Component_iterator<T> const chunk_begin
				{
					chunk.data() + m_component_offset
				};

				Component_iterator<T> const chunk_end
				{
					chunk.data() + m_component_offset + number_of_elements_in_chunk * sizeof(T)
				};

				return {chunk_begin, chunk_end};
			}
			else
			{
				return {};
			}
		}
		
		Chunk_group& m_chunk_group;
		std::size_t m_current_chunk_index;
		std::size_t m_number_of_elements_per_chunk;
		std::size_t m_component_offset;
		Component_chunk_view<T> m_component_chunk_view;

	};

	export template <typename T>
	class Component_chunk_group_view
	{
	public:

		using Iterator = Component_chunk_iterator<T>;

		Component_chunk_group_view(Iterator const begin, Iterator const end) noexcept :
			m_begin{begin},
			m_end{end}
		{
		}

		Iterator begin() const noexcept
		{
			return m_begin;
		}

		Iterator end() const noexcept
		{
			return m_end;
		}

	private:

		Iterator m_begin;
		Iterator m_end;

	};

	export class Component_chunk_group
	{
	public:

		using Index = std::size_t;

		Component_chunk_group(
			std::span<Component_type_info const> const component_type_infos,
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
					std::size_t const chunk_index = chunk_group.number_of_elements / m_number_of_entities_per_chunk;
					Chunk& chunk = chunk_group.chunks.at(chunk_index);

					std::size_t const new_entity_index = chunk_group.number_of_elements;

					{
						std::size_t entity_value_offset = get_component_element_offset(get_entity_component_type_info(), new_entity_index);

						assert((entity_value_offset + sizeof(Entity)) <= chunk.size());
						std::memcpy(chunk.data() + entity_value_offset, &entity, sizeof(Entity));
					}

					for (std::size_t component_type_index = 0; component_type_index + 1 < m_component_type_infos.size(); ++component_type_index)
					{
						Component_type_info const type_info = m_component_type_infos[component_type_index];

						std::size_t component_value_offset = get_component_element_offset(type_info, new_entity_index);

						assert((component_value_offset + type_info.size) <= chunk.size());
						std::memset(chunk.data() + component_value_offset, 0, type_info.size);
					}
					
					++chunk_group.number_of_elements;

					return chunk_group.number_of_elements - 1;
				}
				else
				{
					Chunk new_chunk{m_chunk_allocator};
					new_chunk.resize(get_chunk_size());
					
					std::size_t const new_entity_index = chunk_group.number_of_elements;

					{
						std::size_t entity_value_offset = get_component_element_offset(get_entity_component_type_info(), new_entity_index);

						assert((entity_value_offset + sizeof(Entity)) <= new_chunk.size());
						std::memcpy(new_chunk.data() + entity_value_offset, &entity, sizeof(Entity));
					}

					for (std::size_t component_type_index = 0; component_type_index + 1 < m_component_type_infos.size(); ++component_type_index)
					{
						Component_type_info const type_info = m_component_type_infos[component_type_index];

						std::size_t component_value_offset = get_component_element_offset(type_info, new_entity_index);

						assert((component_value_offset + type_info.size) <= new_chunk.size());
						std::memset(new_chunk.data() + component_value_offset, 0, type_info.size);
					}

					++chunk_group.number_of_elements;

					chunk_group.chunks.push_back(std::move(new_chunk));

					return chunk_group.number_of_elements - 1;
				}
			}
			else
			{
				Chunk new_chunk{m_chunk_allocator};
				new_chunk.resize(get_chunk_size());
								
				std::size_t const new_entity_index = 0;

				{
					std::size_t entity_value_offset = get_component_element_offset(get_entity_component_type_info(), new_entity_index);

					assert((entity_value_offset + sizeof(Entity)) <= new_chunk.size());
					std::memcpy(new_chunk.data() + entity_value_offset, &entity, sizeof(Entity));
				}

				for (std::size_t component_type_index = 0; component_type_index + 1 < m_component_type_infos.size(); ++component_type_index)
				{
					Component_type_info const type_info = m_component_type_infos[component_type_index];

					std::size_t component_value_offset = get_component_element_offset(type_info, new_entity_index);

					assert((component_value_offset + type_info.size) <= new_chunk.size());
					std::memset(new_chunk.data() + component_value_offset, 0, type_info.size);
				}

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

			Component_type_info const component_type_info{get_component_type_id<Component_t>(), sizeof(Component_t)};
			std::size_t const offset = get_component_element_offset(component_type_info, index);

			Component_t value{};

			assert((offset + component_type_info.size) <= chunk.size());
			std::memcpy(&value, chunk.data() + offset, component_type_info.size);

			return value;
		}

		template <Concept::Component Component_t>
		void set_component_value(Chunk_group_hash const chunk_group_hash, Index const index, Component_t const& value) noexcept
		{
			Chunk_group& chunk_group = m_chunk_groups.at(chunk_group_hash);

			std::size_t const chunk_index = index / m_number_of_entities_per_chunk;
			Chunk& chunk = chunk_group.chunks[chunk_index];

			Component_type_info const component_type_info{get_component_type_id<Component_t>(), sizeof(Component_t)};
			std::size_t const offset = get_component_element_offset(component_type_info, index);

			assert((offset + component_type_info.size) <= chunk.size());
			std::memcpy(chunk.data() + offset, &value, component_type_info.size);
		}

		void shrink_to_fit(Chunk_group_hash const chunk_group_hash) noexcept
		{
			Chunk_group& chunk_group = m_chunk_groups.at(chunk_group_hash);

			std::size_t const minimal_number_of_chunks = 
				chunk_group.number_of_elements / m_number_of_entities_per_chunk +
				(chunk_group.number_of_elements % m_number_of_entities_per_chunk != 0 ? 1 : 0);

			chunk_group.chunks.resize(minimal_number_of_chunks);
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

		template <Concept::Component Component_t>
		Component_chunk_view<Component_t> get_view(Chunk_group_hash const chunk_group_hash, std::size_t const chunk_index) noexcept
		{
			Chunk_group& chunk_group = m_chunk_groups.at(chunk_group_hash);
			Chunk& chunk = chunk_group.chunks[chunk_index];

			Component_type_info const component_type_info{get_component_type_id<Component_t>(), sizeof(Component_t)};
			std::size_t const offset = get_component_element_offset(component_type_info, 0);
			
			std::size_t const number_of_elements_in_chunk = number_of_elements(chunk_group.number_of_elements, chunk_index);

			typename Component_chunk_view<Component_t>::Iterator const begin{chunk.data() + offset};
			typename Component_chunk_view<Component_t>::Iterator const end{chunk.data() + offset + number_of_elements_in_chunk * sizeof(Component_t)};

			return {begin, end};
		}

		template <Concept::Component Component_t>
		Component_chunk_group_view<Component_t> get_view(Chunk_group_hash const chunk_group_hash) noexcept
		{
			Chunk_group& chunk_group = m_chunk_groups.at(chunk_group_hash);

			Component_type_info const component_type_info{get_component_type_id<Component_t>(), sizeof(Component_t)};
			std::size_t const component_offset = get_component_element_offset(component_type_info, 0);

			Component_chunk_iterator<Component_t> const begin
			{
				chunk_group,
				0,
				m_number_of_entities_per_chunk,
				component_offset
			};

			Component_chunk_iterator<Component_t> const end
			{
				chunk_group,
				chunk_group.number_of_elements,
				m_number_of_entities_per_chunk,
				component_offset
			};

			return {begin, end};
		}

	private:

		std::size_t number_of_elements(std::size_t const number_of_elements_in_chunk_group, std::size_t const chunk_index) const noexcept
		{
			std::size_t const capacity_at_previous_chunk = chunk_index * m_number_of_entities_per_chunk;
			std::size_t const capacity_at_current_chunk = capacity_at_previous_chunk + m_number_of_entities_per_chunk;

			if (number_of_elements_in_chunk_group >= capacity_at_current_chunk)
			{
				return m_number_of_entities_per_chunk;
			}
			else if (number_of_elements_in_chunk_group > capacity_at_previous_chunk)
			{
				return number_of_elements_in_chunk_group - capacity_at_previous_chunk;
			}
			else
			{
				return 0;
			}
		}

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

		std::size_t get_component_element_offset(
			Component_type_info const component_type_info,
			std::size_t entity_index
		) const noexcept
		{
			Component_type_ID const component_type_id = component_type_info.id;
			std::size_t const component_type_size = component_type_info.size;

			std::size_t const offset = 
				m_number_of_entities_per_chunk * get_component_offset(component_type_id) + 
				(entity_index % m_number_of_entities_per_chunk) * component_type_size;

			return offset;
		}

		Component_type_info get_entity_component_type_info() const noexcept
		{
			return m_component_type_infos.back();
		}

	private:

		std::pmr::unordered_map<Chunk_group_hash, Chunk_group> m_chunk_groups;
		std::pmr::vector<Component_type_info> m_component_type_infos;
		std::size_t m_number_of_entities_per_chunk;
		std::pmr::polymorphic_allocator<std::byte> m_chunk_allocator;

	};
}