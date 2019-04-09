#include <catch2/catch.hpp>

#include <Maia/Utilities/Containers/Chunks/Memory_chunks.hpp>

namespace Maia::Utilities::Test
{
	namespace
	{
		struct Entity
		{
			std::size_t id;
		};

		bool operator==(Entity const& lhs, Entity const& rhs)
		{
			return lhs.id == rhs.id;
		}

		struct Position
		{
			float x, y, z;
		};

		bool operator==(Position const& lhs, Position const& rhs)
		{
			return lhs.x == rhs.x
				&& lhs.y == rhs.y
				&& lhs.z == rhs.z;
		}

		std::ostream& operator<<(std::ostream& output_stream, Position const& value)
		{
			output_stream << "{" << value.x << ", " << value.y << ", " << value.z << "}";
			return output_stream;
		}

		struct Rotation
		{
			float a, b, c, w;
		};

		bool operator==(Rotation const& lhs, Rotation const& rhs)
		{
			return lhs.a == rhs.a
				&& lhs.b == rhs.b
				&& lhs.c == rhs.c
				&& lhs.w == rhs.w;
		}

		std::ostream& operator<<(std::ostream& output_stream, Rotation const& value)
		{
			output_stream << "{" << value.a << ", " << value.b << ", " << value.c << ", " << value.w << "}";
			return output_stream;
		}
	}

	SCENARIO("Create a component group, add, remove and set components", "[Component_group]")
	{
		GIVEN("A component group consisting of Position and Rotation components and capacity per chunk equals 2")
		{
			Component_group<2, Entity, Position, Rotation> component_group;

			THEN("The component group has an initial size of 0")
			{
				CHECK(component_group.size() == 0);
			}

			THEN("The component group has an initial capacity of 0")
			{
				CHECK(component_group.capacity() == 0);
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
					Position const position0{ 1.0f, 2.0f, 3.0f };
					Rotation const rotation0{ 4.0f, 5.0f, 6.0f, 7.0f };
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
						Position const position1{ 8.0f, 9.0f, 10.0f };
						Rotation const rotation1{ 11.0f, 12.0f, 13.0f, 14.0f };
						component_group.push_back(entity1, position1, rotation1);

						Entity const entity2{ 2 };
						Position const position2{ 15.0f, 16.0f, 17.0f };
						Rotation const rotation2{ 18.0f, 19.0f, 20.0f, 21.0f };
						component_group.push_back(entity2, position2, rotation2);

						THEN("The component group size is 3")
						{
							CHECK(component_group.size() == 3);
						}

						THEN("The component group capacity is 4")
						{
							CHECK(component_group.capacity() == 4);
						}

						AND_WHEN("Element 0 is removed")
						{
							Component_group_element_moved<Entity> element_moved_data = component_group.erase({ 0 });

							THEN("The moved element was the element at the back")
							{
								CHECK(element_moved_data.entity == entity2);

								auto [entity, position, rotation] = component_group.get_components_data({ 0 });
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
						}
					}
				}
			}

			WHEN("Pushing back components Position { 2.0f, 0.5f, -1.0f } and Rotation { 0.1f, 0.2f, 0.3f, 1.0f }")
			{
				Entity const original_entity{ 1 };
				Position const original_position{ 2.0f, 0.5f, -1.0f };
				Rotation const original_rotation{ 0.1f, 0.2f, 0.3f, 1.0f };

				auto const index = component_group.push_back(original_entity, original_position, original_rotation);

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
					std::tuple<Entity, Position, Rotation> const components = component_group.get_components_data(index);
					Entity const& current_entity = std::get<0>(components);
					Position const& current_position = std::get<1>(components);
					Rotation const& current_rotation = std::get<2>(components);

					THEN("The entity, position and rotation components should have the same value that was pushed back")
					{
						CHECK(current_entity == original_entity);
						CHECK(current_position == original_position);
						CHECK(current_rotation == original_rotation);
					}
				}

				AND_WHEN("Setting element at index 0 given Entity { 3 } Position {-1.0f, 2.0f, 3.0f } and Rotation { 0.5f, 0.5f, 0.0f, 0.5f }")
				{
					Entity const new_entity{ 3 };
					Position const new_position{ -1.0f, 2.0f, 3.0f };
					Rotation const new_rotation{ 0.5f, 0.5f, 0.0f, 0.5f };

					component_group.set_components_data(index, new_entity, new_position, new_rotation);

					AND_WHEN("Getting components of element at index 0")
					{
						std::tuple<Entity, Position, Rotation> const components = component_group.get_components_data(index);
						Entity const& current_entity = std::get<0>(components);
						Position const& current_position = std::get<1>(components);
						Rotation const& current_rotation = std::get<2>(components);

						THEN("The position and rotation components should have the same value that was set")
						{
							CHECK(current_entity == new_entity);
							CHECK(current_position == new_position);
							CHECK(current_rotation == new_rotation);
						}
					}
				}

				AND_WHEN("Setting element at index 0 given Position { 1.0f, 4.0f, 2.0f  }")
				{
					Position new_position{ 1.0f, 4.0f, 2.0f };

					component_group.set_component_data(index, new_position);

					AND_WHEN("Getting position of element at index 0")
					{
						Position const current_position = component_group.get_component_data<Position>(index);

						THEN("The position component should have the same value that was set")
						{
							CHECK(current_position == new_position);
						}
					}
				}

				AND_WHEN("Setting element at index 0 given Rotation { 1.0f, 2.0f, 3.0f, 4.0f }")
				{
					Rotation new_rotation{ 1.0f, 2.0f, 3.0f, 4.0f };

					component_group.set_component_data(index, new_rotation);

					AND_WHEN("Getting rotation of element at index 0")
					{
						Rotation const current_rotation = component_group.get_component_data<Rotation>(index);

						THEN("The rotation component should have the same value that was set")
						{
							CHECK(current_rotation == new_rotation);
						}
					}
				}

				AND_WHEN("Popping back")
				{
					std::tuple<Entity, Position, Rotation> const components = component_group.pop_back();
					Entity const& current_entity = std::get<0>(components);
					Position const& current_position = std::get<1>(components);
					Rotation const& current_rotation = std::get<2>(components);

					THEN("The entity, position and rotation components should have the same value that was pushed back")
					{
						CHECK(current_entity == original_entity);
						CHECK(current_position == original_position);
						CHECK(current_rotation == original_rotation);
					}
				}
			}
		}
	}
}
