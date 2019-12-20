import maia.ecs.component;
import maia.ecs.component_group;
import maia.ecs.components_chunk;
import maia.ecs.entity;
import maia.ecs.components.local_position;
import maia.ecs.components.local_rotation;

import <catch2/catch.hpp>;

import <Eigen/Geometry>;

import <array>;
import <memory_resource>;
import <optional>;
import <span>;

namespace Maia::ECS::Test
{
	using namespace Maia::ECS::Components;

	SCENARIO("Create a component group, add, remove and set components", "[Component_group]")
	{
		GIVEN("A component group consisting of Local_position and Local_rotation components and capacity per chunk equals 2 elements")
		{
			Component_group component_group{ make_component_group<Entity, Local_position, Local_rotation>(2) };

			THEN("The component group has an initial size of 0")
			{
				CHECK(component_group.size() == 0);
			}

			THEN("The component group has an initial capacity of 0")
			{
				CHECK(component_group.capacity() == 0);
			}

			WHEN("Pushing back components Local_position { 2.0f, 0.5f, -1.0f } and Local_rotation { 0.1f, 0.2f, 0.3f, 1.0f }")
			{
				Entity const original_entity{ 1 };
				Local_position const original_position{ { 2.0f, 0.5f, -1.0f } };
				Local_rotation const original_rotation{ { 0.1f, 0.2f, 0.3f, 1.0f } };

				Component_group::Index const index = component_group.push_back(original_entity, original_position, original_rotation);

				THEN("The returned index should be equals to 0")
				{
					CHECK(index.value == 0);
				}

				THEN("The size of the chunk should have been incremented by 1")
				{
					CHECK(component_group.size() == 1);
				}

				AND_WHEN("Getting components of element at index 0")
				{
					std::tuple<Entity, Local_position, Local_rotation> const components = component_group.get_components_data<Entity, Local_position, Local_rotation>(index);
					Entity const& current_entity = std::get<0>(components);
					Local_position const& current_position = std::get<1>(components);
					Local_rotation const& current_rotation = std::get<2>(components);

					THEN("The entity, position and rotation components should have the same value that was pushed back")
					{
						CHECK(current_entity == original_entity);
						CHECK(current_position == original_position);
						CHECK(current_rotation == original_rotation);
					}
				}

				AND_WHEN("Setting element at index 0 given Entity { 3 } Local_position {-1.0f, 2.0f, 3.0f } and Local_rotation { 0.5f, 0.5f, 0.0f, 0.5f }")
				{
					Entity const new_entity{ 3 };
					Local_position const new_position{ { -1.0f, 2.0f, 3.0f } };
					Local_rotation const new_rotation{ { 0.5f, 0.5f, 0.0f, 0.5f } };

					component_group.set_components_data(index, new_entity, new_position, new_rotation);

					AND_WHEN("Getting components of element at index 0")
					{
						std::tuple<Entity, Local_position, Local_rotation> const components = component_group.get_components_data<Entity, Local_position, Local_rotation>(index);
						Entity const& current_entity = std::get<0>(components);
						Local_position const& current_position = std::get<1>(components);
						Local_rotation const& current_rotation = std::get<2>(components);

						THEN("The position and rotation components should have the same value that was set")
						{
							CHECK(current_entity == new_entity);
							CHECK(current_position == new_position);
							CHECK(current_rotation == new_rotation);
						}
					}
				}

				AND_WHEN("Setting element at index 0 given Local_position { 1.0f, 4.0f, 2.0f  }")
				{
					Local_position new_position{ { 1.0f, 4.0f, 2.0f } };

					component_group.set_component_data(index, new_position);

					AND_WHEN("Getting position of element at index 0")
					{
						Local_position const current_position = 
							component_group.get_component_data<Local_position>(index).get();

						THEN("The position component should have the same value that was set")
						{
							CHECK(current_position == new_position);
						}
					}
				}

				AND_WHEN("Setting element at index 0 given Local_rotation { 1.0f, 2.0f, 3.0f, 4.0f }")
				{
					Local_rotation new_rotation{ { 1.0f, 2.0f, 3.0f, 4.0f } };

					component_group.set_component_data(index, new_rotation);

					AND_WHEN("Getting rotation of element at index 0")
					{
						Local_rotation const current_rotation = 
							component_group.get_component_data<Local_rotation>(index).get();

						THEN("The rotation component should have the same value that was set")
						{
							CHECK(current_rotation == new_rotation);
						}
					}
				}

				AND_WHEN("Popping back")
				{
					std::tuple<Entity, Local_position, Local_rotation> const components = component_group.back<Entity, Local_position, Local_rotation>();
					Entity const& current_entity = std::get<0>(components);
					Local_position const& current_position = std::get<1>(components);
					Local_rotation const& current_rotation = std::get<2>(components);
					component_group.pop_back();

					THEN("The entity, position and rotation components should have the same value that was pushed back")
					{
						CHECK(current_entity == original_entity);
						CHECK(current_position == original_position);
						CHECK(current_rotation == original_rotation);
					}
				}
			}

			WHEN("Reserving memory on component group so that it can hold at least 3 elements")
			{
				component_group.reserve(3);

				THEN("The component group size is 0")
				{
					CHECK(component_group.size() == 0);
				}

				THEN("The component group capacity is 4")
				{
					CHECK(component_group.capacity() == 4);
				}

				AND_WHEN("An element is pushed to the component group")
				{
					Entity const entity0{ 0 };
					Local_position const position0{ { 1.0f, 2.0f, 3.0f } };
					Local_rotation const rotation0{ { 4.0f, 5.0f, 6.0f, 7.0f } };
					component_group.push_back(entity0, position0, rotation0);

					THEN("The component group size is 1")
					{
						CHECK(component_group.size() == 1);
					}

					THEN("The component group capacity is 4")
					{
						CHECK(component_group.capacity() == 4);
					}

					AND_WHEN("The element is removed from the component group")
					{
						component_group.pop_back();

						THEN("The component group size is 0")
						{
							CHECK(component_group.size() == 0);
						}

						THEN("The component group capacity is 4")
						{
							CHECK(component_group.capacity() == 4);
						}
					}

					AND_WHEN("The component group shrinks to fit")
					{
						component_group.shrink_to_fit();

						THEN("The component group size is 1")
						{
							CHECK(component_group.size() == 1);
						}

						THEN("The component group capacity is 2")
						{
							CHECK(component_group.capacity() == 2);
						}
					}

					AND_WHEN("Two more elements are added")
					{
						Entity const entity1{ 1 };
						Local_position const position1{ { 8.0f, 9.0f, 10.0f } };
						Local_rotation const rotation1{ { 11.0f, 12.0f, 13.0f, 14.0f } };
						component_group.push_back(entity1, position1, rotation1);

						Entity const entity2{ 2 };
						Local_position const position2{ { 15.0f, 16.0f, 17.0f } };
						Local_rotation const rotation2{ { 18.0f, 19.0f, 20.0f, 21.0f } };
						component_group.push_back(entity2, position2, rotation2);

						THEN("The component group size is 3")
						{
							CHECK(component_group.size() == 3);
						}

						THEN("The component group capacity is 4")
						{
							CHECK(component_group.capacity() == 4);
						}

						AND_WHEN("Element 0 is erased")
						{
							std::optional<Component_group_entity_moved> entity_moved_data = component_group.erase({ 0 });

							THEN("The moved element was the element at the back")
							{
								REQUIRE(entity_moved_data.has_value());
								CHECK(entity_moved_data.value().entity == entity2);

								auto[entity, position, rotation] = component_group.get_components_data<Entity, Local_position, Local_rotation>({ 0 });
								CHECK(entity == entity2);
								CHECK(position == position2);
								CHECK(rotation == rotation2);
							}

							THEN("The component group size is 2")
							{
								CHECK(component_group.size() == 2);
							}

							THEN("The component group capacity is 4")
							{
								CHECK(component_group.capacity() == 4);
							}

							AND_WHEN("The component group shrinks to fit")
							{
								component_group.shrink_to_fit();

								THEN("The component group size is 2")
								{
									CHECK(component_group.size() == 2);
								}

								THEN("The component group capacity is 2")
								{
									CHECK(component_group.capacity() == 2);
								}
							}

							AND_WHEN("Removing the remaining elements")
							{
								std::optional<Component_group_entity_moved> entity_moved_data_1 = component_group.erase({ 0 });
								std::optional<Component_group_entity_moved> entity_moved_data_2 = component_group.erase({ 0 });

								THEN("One of the elements should be moved, and the last one not")
								{
									CHECK(entity_moved_data_1.has_value());
									CHECK(!entity_moved_data_2.has_value());
								}
							}
						}
					}
				}
			}
		}
	}
}
