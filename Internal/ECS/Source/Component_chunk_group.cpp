module maia.ecs.component_chunk_group;

import maia.ecs.component;
import maia.ecs.entity;

import <cassert>;
import <cstddef>;
import <cstring>;
import <memory_resource>;
import <optional>;
import <span>;
import <vector>;

namespace Maia::ECS
{
	namespace
	{
		std::size_t calculate_size_of_single_element(std::span<Component_type_info const> const component_type_infos) noexcept
		{
			std::size_t size_of_single_element{ 0 };

			for (Component_type_info const& component_type_info : component_type_infos)
			{
				size_of_single_element += component_type_info.size;
			}

			return size_of_single_element;
		}

		std::pmr::vector<Component_type_info_and_offset> create_component_type_infos(
			std::span<Component_type_info const> const component_type_infos, 
			std::size_t const capacity_per_chunk,
			std::pmr::polymorphic_allocator<Component_type_info_and_offset> const& type_infos_allocator
		) noexcept
		{
			std::pmr::vector<Component_type_info_and_offset> type_infos_and_offset{type_infos_allocator};
			type_infos_and_offset.reserve(component_type_infos.size());

			std::size_t current_offset{ 0 };

			for (Component_type_info const& component_type_info : component_type_infos)
			{
				type_infos_and_offset.push_back({ component_type_info.id, current_offset, component_type_info.size });

				current_offset += capacity_per_chunk * component_type_info.size;
			}

			return type_infos_and_offset;
		}
	}

	Component_chunk_group::Component_chunk_group(
		std::span<Component_type_info const> const component_type_infos,
		std::size_t const capacity_per_chunk,
		std::pmr::polymorphic_allocator<Component_chunk> chunks_allocator,
		std::pmr::polymorphic_allocator<Component_type_info_and_offset> type_infos_allocator
	) noexcept :
		m_size{ 0 },
		m_size_of_single_element{ calculate_size_of_single_element(component_type_infos) },
		m_capacity_per_chunk{ capacity_per_chunk },
		m_chunks{std::move(chunks_allocator)},
		m_component_type_infos{ create_component_type_infos(component_type_infos, m_capacity_per_chunk, std::move(type_infos_allocator)) }
	{
	}



	std::size_t Component_chunk_group::size() const noexcept
	{
		return m_size;
	}

	std::size_t Component_chunk_group::num_chunks() const noexcept
	{
		return m_chunks.size();
	}

	void Component_chunk_group::reserve(std::size_t const new_capacity) noexcept
	{
		std::size_t const number_of_chunks = new_capacity / m_capacity_per_chunk
			+ (new_capacity % m_capacity_per_chunk > 0 ? 1 : 0);

		m_chunks.reserve(number_of_chunks);

		while (m_chunks.size() < number_of_chunks)
		{
			Component_chunk& chunk = m_chunks.emplace_back();
			chunk.resize(m_capacity_per_chunk * m_size_of_single_element, std::byte{});
		}
	}

	std::size_t Component_chunk_group::capacity() const noexcept
	{
		return m_chunks.size() * m_capacity_per_chunk;
	}

	void Component_chunk_group::shrink_to_fit() noexcept
	{
		std::size_t const ideal_number_of_chunks = size() / m_capacity_per_chunk
			+ (size() % m_capacity_per_chunk > 0 ? 1 : 0);

		m_chunks.erase(m_chunks.begin() + ideal_number_of_chunks, m_chunks.end());

		m_chunks.shrink_to_fit();
	}



	std::optional<Component_group_entity_moved> Component_chunk_group::erase(Index const index) noexcept
	{
		if (index.value < m_size - 1)
		{
			std::size_t const chunk_to_delete_from_index = index.value / m_capacity_per_chunk;
			Component_chunk& chunk_to_delete_from = m_chunks[chunk_to_delete_from_index];
			std::size_t const entity_to_delete_index = calculate_entity_index(index);

			Component_chunk const& chunk_to_copy_from = m_chunks.back();
			std::size_t const entity_to_copy_index = m_size - m_capacity_per_chunk * (m_chunks.size() - 1) - 1;

			for (Component_type_info_and_offset const type_info : m_component_type_infos)
			{
				std::size_t const component_offset = type_info.offset;
				std::size_t const component_size = type_info.size;

				std::byte* component_to_overwrite = chunk_to_delete_from.data() + component_offset + entity_to_delete_index * component_size;
				std::byte const* component_to_copy = chunk_to_copy_from.data() + component_offset + entity_to_copy_index * component_size;

				std::memcpy(
					component_to_overwrite,
					component_to_copy,
					component_size
				);
			}

			decrement_size();

			Component_view<Entity const> const entity_view =
				get_component_data<Entity>(index);

			return Element_moved{ entity_view.get() };
		}

		else
		{
			decrement_size();

			return {};
		}
	}

	Component_chunk_group::Index Component_chunk_group::push_back() noexcept
	{
		if (size() == capacity())
		{
			reserve(capacity() + m_capacity_per_chunk);
		}

		Index const index = { size() };

		increment_size();

		return index;
	}

	void Component_chunk_group::pop_back() noexcept
	{
		decrement_size();
	}



	std::byte const* Component_chunk_group::get_component_data_impl(Component_type_ID const component_id, Index const index) const noexcept
	{
		Component_chunk const& chunk = get_entity_chunk(index);

		Component_type_info_and_offset const type_info = get_component_type_info(component_id);
		std::size_t const entity_index = calculate_entity_index(index);

		return chunk.data() + type_info.offset + entity_index * type_info.size;
	}

	std::byte* Component_chunk_group::get_component_data_impl(Component_type_ID const component_id, Index const index) noexcept
	{
		Component_chunk& chunk = get_entity_chunk(index);

		Component_type_info_and_offset const type_info = get_component_type_info(component_id);
		std::size_t const entity_index = calculate_entity_index(index);

		return chunk.data() + type_info.offset + entity_index * type_info.size;
	}







	void Component_chunk_group::increment_size() noexcept
	{
		++m_size;
	}
	void Component_chunk_group::decrement_size() noexcept
	{
		assert(m_size > 0);
		--m_size;
	}

	Component_chunk const& Component_chunk_group::get_entity_chunk(Component_group_entity_index const component_group_index) const noexcept
	{
		return m_chunks[component_group_index.value / m_capacity_per_chunk];
	}
	Component_chunk& Component_chunk_group::get_entity_chunk(Component_group_entity_index const component_group_index) noexcept
	{
		return m_chunks[component_group_index.value / m_capacity_per_chunk];
	}

	std::size_t Component_chunk_group::calculate_entity_index(Component_group_entity_index const component_group_index) const noexcept
	{
		return component_group_index.value % m_capacity_per_chunk;
	}

	std::size_t Component_chunk_group::get_component_offset(Component_type_ID const component_id) const noexcept
	{
		auto const component_offset_and_size = std::find_if(m_component_type_infos.begin(), m_component_type_infos.end(),
			[&](Component_type_info_and_offset const& type_info) -> bool {  return type_info.id == component_id; });

		return component_offset_and_size->offset;
	}

	Component_type_info_and_offset Component_chunk_group::get_component_type_info(Component_type_ID const component_id) const noexcept
	{
		auto const component_offset_and_size = std::find_if(m_component_type_infos.begin(), m_component_type_infos.end(),
			[&](Component_type_info_and_offset const& type_info) -> bool {  return type_info.id == component_id; });

		return *component_offset_and_size;
	}
}