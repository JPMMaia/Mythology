#include <Maia/GameEngine/Systems/Transform_system.hpp>

#include <iostream>

namespace Maia::GameEngine::Systems
{
	Transform_matrix create_transform(Local_position const& position, Local_rotation const& rotation)
	{
		Eigen::Vector3f const& translation = position.value;
		Eigen::Matrix3f const rotation_matrix = rotation.value.matrix();

		Eigen::Matrix4f matrix;
		matrix <<
			rotation_matrix(0, 0), rotation_matrix(0, 1), rotation_matrix(0, 2), translation(0),
			rotation_matrix(1, 0), rotation_matrix(1, 1), rotation_matrix(1, 2), translation(1),
			rotation_matrix(2, 0), rotation_matrix(2, 1), rotation_matrix(2, 2), translation(2),
			0.0f, 0.0f, 0.0f, 1.0f;

		return { matrix };
	}

	Transforms_tree create_transforms_tree(
		Entity_manager const& entity_manager,
		Entity root_transform_entity
	)
	{
		Transforms_tree transforms_tree;

		gsl::span<Component_group_mask const> const component_types_groups =
			entity_manager.get_component_types_groups();

		gsl::span<Component_group const> const component_groups =
			entity_manager.get_component_groups();

		for (std::ptrdiff_t component_group_index = 0; component_group_index < component_groups.size(); ++component_group_index)
		{
			Component_group_mask const component_types = component_types_groups[component_group_index];

			if (component_types.contains<Transform_root, Transform_parent>())
			{
				Component_group const& component_group = component_groups[component_group_index];

				for (std::size_t chunk_index = 0; chunk_index < component_group.num_chunks(); ++chunk_index)
				{
					gsl::span<Transform_root const> roots = component_group.components<Transform_root>(chunk_index);
					gsl::span<Transform_parent const> parents = component_group.components<Transform_parent>(chunk_index);
					gsl::span<Entity const> entities = component_group.components<Entity>(chunk_index);

					for (std::ptrdiff_t component_index = 0; component_index < roots.size(); ++component_index)
					{
						if (roots[component_index].entity == root_transform_entity)
						{
							transforms_tree.insert(std::make_pair(parents[component_index], entities[component_index]));
						}
					}
				}
			}
		}

		return transforms_tree;
	}

	namespace
	{
		using transforms_tree_iterator = std::unordered_multimap<Transform_parent, Entity>::const_iterator;

		void update_child_transforms_aux(
			Entity_manager& entity_manager,
			Transforms_tree const& transforms_tree,
			Entity root_transform_entity,
			Transform_matrix const& root_transform_matrix,
			std::pair<transforms_tree_iterator, transforms_tree_iterator> children_range
		)
		{
			for (auto it = children_range.first; it != children_range.second; ++it)
			{
				Entity const child_entity = it->second;

				auto const[position, rotation] = entity_manager.get_components_data<Local_position, Local_rotation>(child_entity);

				Transform_matrix const local_transform_matrix = create_transform(position, rotation);

				Transform_matrix const world_transform_matrix{ root_transform_matrix.value * local_transform_matrix.value };
				entity_manager.set_component_data(child_entity, world_transform_matrix);

				auto const children_of_child_range = transforms_tree.equal_range({ child_entity });
				update_child_transforms_aux(entity_manager, transforms_tree, child_entity, world_transform_matrix, children_of_child_range);
			}
		}
	}

	void update_child_transforms(
		Entity_manager& entity_manager,
		Transforms_tree const& transforms_tree,
		Entity root_transform_entity,
		Transform_matrix const& root_transform_matrix
	)
	{
		auto const children_range = transforms_tree.equal_range({ root_transform_entity });
		update_child_transforms_aux(entity_manager, transforms_tree, root_transform_entity, root_transform_matrix, children_range);
	}

	namespace
	{
		void update_transform_tree(Entity_manager& entity_manager, Entity const root_entity, Local_position const root_position, Local_rotation const root_rotation)
		{
			Transform_matrix const root_transform = create_transform(root_position, root_rotation);
			entity_manager.set_component_data(root_entity, root_transform);

			Transforms_tree const transforms_tree = create_transforms_tree(entity_manager, root_entity);

			update_child_transforms(entity_manager, transforms_tree, root_entity, root_transform);
		}
	}
	void Transform_system::execute(Entity_manager& entity_manager)
	{
		gsl::span<Component_group_mask const> const component_types_groups =
			entity_manager.get_component_types_groups();

		gsl::span<Component_group> const component_groups =
			entity_manager.get_component_groups();

		for (std::ptrdiff_t component_group_index = 0; component_group_index < component_groups.size(); ++component_group_index)
		{
			Component_group_mask const component_types = component_types_groups[component_group_index];

			if (component_types.contains<Transform_tree_dirty>())
			{
				Component_group& component_group = component_groups[component_group_index];

				for (std::size_t chunk_index = 0; chunk_index < component_group.num_chunks(); ++chunk_index)
				{
					gsl::span<Entity const> entities
						= component_group.components<Entity>(chunk_index);

					gsl::span<Local_position const> positions
						= component_group.components<Local_position>(chunk_index);

					gsl::span<Local_rotation const> rotations
						= component_group.components<Local_rotation>(chunk_index);

					gsl::span<Transform_tree_dirty> transform_trees_dirty 
						= component_group.components<Transform_tree_dirty>(chunk_index);

					for (std::ptrdiff_t component_index = 0; component_index < transform_trees_dirty.size(); ++component_index)
					{
						if (transform_trees_dirty[component_index].value)
						{
							// TODO Create a new thread
							{
								update_transform_tree(entity_manager, entities[component_index], positions[component_index], rotations[component_index]);
							}

							transform_trees_dirty[component_index].value = false;
						}
					}
				}
			}
		}
	}
}
