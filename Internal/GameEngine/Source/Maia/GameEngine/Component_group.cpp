#include "Component_group.hpp"

#include <cassert>
#include <optional>

#include <gsl/span>

#include <Maia/GameEngine/Components_chunk.hpp>

namespace Maia::GameEngine
{
	namespace
	{
		std::size_t calculate_size_of_single_element(gsl::span<Component_info const> const component_infos)
		{
			std::size_t size_of_single_element{ 0 };

			for (Component_info const& component_info : component_infos)
			{
				size_of_single_element += component_info.size.value;
			}

			return size_of_single_element;
		}

		std::vector<Component_type_info> create_component_type_infos(
			gsl::span<Component_info const> const component_infos, 
			std::size_t const capacity_per_chunk
		)
		{
			std::vector<Component_type_info> type_infos;
			type_infos.reserve(component_infos.size());

			std::size_t current_offset{ 0 };

			for (Component_info const& component_info : component_infos)
			{
				type_infos.push_back({ component_info.id, { current_offset }, component_info.size });

				current_offset += capacity_per_chunk * component_info.size.value;
			}

			return type_infos;
		}
	}

	Component_group::Component_group(
		gsl::span<Component_info const> const component_infos,
		std::size_t const capacity_per_chunk
	) :
		m_size{ 0 },
		m_size_of_single_element{ calculate_size_of_single_element(component_infos) },
		m_capacity_per_chunk{ capacity_per_chunk },
		m_chunks{},
		m_component_type_infos{ create_component_type_infos(component_infos, m_capacity_per_chunk) }
	{
	}



	std::size_t Component_group::size() const
	{
		return m_size;
	}

	std::size_t Component_group::num_chunks() const
	{
		return m_chunks.size();
	}

	void Component_group::reserve(std::size_t const new_capacity)
	{
		std::size_t const number_of_chunks = new_capacity / m_capacity_per_chunk
			+ (new_capacity % m_capacity_per_chunk > 0 ? 1 : 0);

		m_chunks.reserve(number_of_chunks);

		while (m_chunks.size() < number_of_chunks)
		{
			Components_chunk& chunk = m_chunks.emplace_back();
			chunk.resize(m_capacity_per_chunk * m_size_of_single_element, std::byte{});
		}
	}

	std::size_t Component_group::capacity() const
	{
		return m_chunks.size() * m_capacity_per_chunk;
	}

	void Component_group::shrink_to_fit()
	{
		std::size_t const ideal_number_of_chunks = size() / m_capacity_per_chunk
			+ (size() % m_capacity_per_chunk > 0 ? 1 : 0);

		m_chunks.erase(m_chunks.begin() + ideal_number_of_chunks, m_chunks.end());

		m_chunks.shrink_to_fit();
	}



	std::optional<Component_group_entity_moved> Component_group::erase(Index index)
	{
		if (index.value < m_size - 1)
		{
			std::size_t const chunk_to_delete_from_index = index.value / m_capacity_per_chunk;
			Components_chunk& chunk_to_delete_from = m_chunks[chunk_to_delete_from_index];
			std::size_t const entity_to_delete_index = calculate_entity_index(index);

			Components_chunk const& chunk_to_copy_from = m_chunks.back();
			std::size_t const entity_to_copy_index = m_size - m_capacity_per_chunk * (m_chunks.size() - 1) - 1;

			for (Component_type_info const type_info : m_component_type_infos)
			{
				std::size_t const component_offset = type_info.offset;
				std::size_t const component_size = type_info.size.value;

				std::byte* component_to_overwrite = chunk_to_delete_from.data() + component_offset + entity_to_delete_index * component_size;
				std::byte const* component_to_copy = chunk_to_copy_from.data() + component_offset + entity_to_copy_index * component_size;

				std::memcpy(
					component_to_overwrite,
					component_to_copy,
					component_size
				);
			}

			decrement_size();

			Entity const entity =
				get_component_data<Entity>(index);

			return Element_moved{ entity };
		}

		else
		{
			decrement_size();

			return {};
		}
	}

	Component_group::Index Component_group::push_back()
	{
		if (size() == capacity())
		{
			reserve(capacity() + m_capacity_per_chunk);
		}

		Index const index = { size() };

		increment_size();

		return index;
	}

	void Component_group::pop_back()
	{
		decrement_size();
	}



	std::byte const* Component_group::get_component_data_impl(Component_ID const component_id, Index index) const
	{
		Components_chunk const& chunk = get_entity_chunk(index);

		Component_type_info const type_info = get_component_type_info(component_id);
		std::size_t const entity_index = calculate_entity_index(index);

		return chunk.data() + type_info.offset + entity_index * type_info.size.value;
	}

	std::byte* Component_group::get_component_data_impl(Component_ID const component_id, Index index)
	{
		Components_chunk& chunk = get_entity_chunk(index);

		Component_type_info const type_info = get_component_type_info(component_id);
		std::size_t const entity_index = calculate_entity_index(index);

		return chunk.data() + type_info.offset + entity_index * type_info.size.value;
	}
	

	

	


	void Component_group::increment_size()
	{
		++m_size;
	}
	void Component_group::decrement_size()
	{
		assert(m_size > 0);
		--m_size;
	}

	Components_chunk const& Component_group::get_entity_chunk(Component_group_entity_index component_group_index) const
	{
		return m_chunks[component_group_index.value / m_capacity_per_chunk];
	}
	Components_chunk& Component_group::get_entity_chunk(Component_group_entity_index component_group_index)
	{
		return m_chunks[component_group_index.value / m_capacity_per_chunk];
	}

	std::size_t Component_group::calculate_entity_index(Component_group_entity_index component_group_index)  const
	{
		return component_group_index.value % m_capacity_per_chunk;
	}

	std::size_t Component_group::get_component_offset(Component_ID const component_id) const
	{
		auto component_offset_and_size = std::find_if(m_component_type_infos.begin(), m_component_type_infos.end(),
			[&](Component_type_info const& type_info) -> bool {  return type_info.id == component_id; });

		return component_offset_and_size->offset;
	}

	Component_type_info Component_group::get_component_type_info(Component_ID const component_id) const
	{
		auto component_offset_and_size = std::find_if(m_component_type_infos.begin(), m_component_type_infos.end(),
			[&](Component_type_info const& type_info) -> bool {  return type_info.id == component_id; });

		return *component_offset_and_size;
	}
}
