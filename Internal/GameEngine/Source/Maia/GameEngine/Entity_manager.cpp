#include "Entity_manager.hpp"

#include <optional>

namespace Maia::GameEngine
{
	Entity_type_id Entity_manager::create_entity_type(
		std::size_t const capacity_per_chunk,
		gsl::span<Component_info const> const component_infos,
		Space const space
	)
	{
		Component_group_mask const component_types_mask = [&component_infos]() -> Component_group_mask
		{
			Component_group_mask component_types_mask = {};

			for (Maia::GameEngine::Component_info const& component_info : component_infos)
			{
				component_types_mask.value.set(component_info.id.value);
			}

			return component_types_mask;
		}();

		assert(component_types_mask.contains<Entity>());

		std::optional<std::size_t> const match_index = [this, &space, &component_types_mask]() -> std::optional<std::size_t>
		{
			for (std::size_t index = 0; index < m_entity_type_ids.size(); ++index)
			{
				if (m_component_types_spaces[index] == space && m_component_group_masks[index] == component_types_mask)
				{
					return index;
				}
			}

			return {};
		}();


		if (match_index)
		{
			return m_entity_type_ids[*match_index];
		}
		else
		{
			m_component_types_spaces.push_back(space);
			m_component_group_masks.push_back(component_types_mask);

			m_component_groups.emplace_back(component_infos, capacity_per_chunk);

			Entity_type_id const entity_type_id{ m_entity_type_ids.size() };
			m_entity_type_ids.push_back(entity_type_id);

			return entity_type_id;
		}
	}
	Entity Entity_manager::create_entity(Entity_type_id entity_type_id)
	{
		assert(m_entity_type_indices.size() < std::numeric_limits<Entity::Integral_type>::max());

		const Entity_type_index entity_type_index = [&]() -> Entity_type_index
		{
			const auto entity_type_id_location = std::find(m_entity_type_ids.begin(), m_entity_type_ids.end(), entity_type_id);

			return { static_cast<std::size_t>(std::distance(m_entity_type_ids.begin(), entity_type_id_location)) };
		}();

		if (!m_deleted_entities.empty())
		{
			Entity const entity = m_deleted_entities.back();
			m_deleted_entities.pop_back();
			m_entities_existence[entity.value] = true;

			m_entity_type_indices[entity.value] = entity_type_index;

			Component_group& component_group = m_component_groups[entity_type_index.value];
			Component_group_entity_index component_group_index = component_group.push_back(entity);
			m_component_group_indices[entity.value] = component_group_index;

			return entity;
		}
		else
		{
			Entity entity
			{
				static_cast<Entity::Integral_type>(m_entity_type_indices.size())
			};
			m_entities_existence.push_back(true);

			m_entity_type_indices.push_back(entity_type_index);

			Component_group& component_group = m_component_groups[entity_type_index.value];
			Component_group_entity_index component_group_index = component_group.push_back(entity);
			m_component_group_indices.push_back(component_group_index);

			return entity;
		}
	}

	void Entity_manager::destroy_entity(Entity entity)
	{
		m_entities_existence[entity.value] = false;
		m_deleted_entities.push_back(entity);

		Entity_type_index const entity_type_index = m_entity_type_indices[entity.value];
		Component_group_entity_index const component_group_index = m_component_group_indices[entity.value];

		if (auto const element_moved = m_component_groups[entity_type_index.value].erase(component_group_index))
		{
			m_component_group_indices[element_moved->entity.value] = component_group_index;
		}
	}

	bool Entity_manager::exists(Entity entity) const
	{
		return m_entities_existence[entity.value];
	}
}