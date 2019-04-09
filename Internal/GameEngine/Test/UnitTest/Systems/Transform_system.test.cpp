#include <catch2/catch.hpp>

#include <Maia/GameEngine/Systems/Transform_system.hpp>

namespace Maia::GameEngine::Systems::Test
{
	SCENARIO("Create transforms")
	{
		GIVEN("Local_position = {} and Local_rotation = {}")
		{
			Local_position const position{};
			Local_rotation const rotation{};

			THEN("The corresponding transform matrix should be equal to the identity matrix")
			{
				Transform_matrix const transform = create_transform(position, rotation);

				CHECK(transform.value.isApprox(Eigen::Matrix4f::Identity(), 0.0f));
			}
		}

		GIVEN("Local_position = { 1.0f, 2.0f, 3.0f }, Local_rotation = { .w = 0.0f, .i = 1.0f, .j = 0.0f, .k = 0.0f }")
		{
			Local_position const position{ { 1.0f, 2.0f, 3.0f } };
			Local_rotation const rotation{ { 0.0f, 1.0f, 0.0f, 0.0f } };

			THEN("The corresponding transform matrix should be equal to the identity matrix")
			{
				Transform_matrix const transform = create_transform(position, rotation);

				Eigen::Matrix4f expected_matrix;
				expected_matrix <<
					1.0f, 0.0f, 0.0f, 1.0f,
					0.0f, -1.0f, 0.0f, 2.0f,
					0.0f, 0.0f, -1.0f, 3.0f,
					0.0f, 0.0f, 0.0f, 1.0f;

				CHECK(transform.value.isApprox(expected_matrix, 0.0f));
			}
		}

		GIVEN("Local_position = { 1.0f, 2.0f, 3.0f }, Local_rotation = { .w = 0.0f, .i = 0.0f, .j = 1.0f, .k = 0.0f }")
		{
			Local_position const position{ { 1.0f, 2.0f, 3.0f } };
			Local_rotation const rotation{ { 0.0f, 0.0f, 1.0f, 0.0f } };

			THEN("The corresponding transform matrix should be equal to the identity matrix")
			{
				Transform_matrix const transform = create_transform(position, rotation);

				Eigen::Matrix4f expected_matrix;
				expected_matrix <<
					-1.0f, 0.0f, 0.0f, 1.0f,
					0.0f, 1.0f, 0.0f, 2.0f,
					0.0f, 0.0f, -1.0f, 3.0f,
					0.0f, 0.0f, 0.0f, 1.0f;

				CHECK(transform.value.isApprox(expected_matrix, 0.0f));
			}
		}

		GIVEN("Local_position = { 1.0f, 2.0f, 3.0f }, Local_rotation = { .w = 0.0f, .i = 0.0f, .j = 1.0f, .k = 0.0f }")
		{
			Local_position const position{ { 1.0f, 2.0f, 3.0f } };

			float const angle = std::acos(-1.0f) / 4.0f;
			Local_rotation const rotation{ { std::cos(angle), 0.0f, 0.0f, std::sin(angle) } };

			THEN("The corresponding transform matrix should be equal to the identity matrix")
			{
				Transform_matrix const transform = create_transform(position, rotation);

				Eigen::Matrix4f expected_matrix;
				expected_matrix <<
					0.0f, -1.0f, 0.0f, 1.0f,
					1.0f, 0.0f, 0.0f, 2.0f,
					0.0f, 0.0f, 1.0f, 3.0f,
					0.0f, 0.0f, 0.0f, 1.0f;

				CHECK(transform.value.isApprox(expected_matrix));
			}
		}
	}

	SCENARIO("Create transform trees")
	{
		GIVEN("An entity manager")
		{
			Entity_manager entity_manager{};

			AND_GIVEN("A Root_transform_entity_type <Local_position, Local_rotation, Transform_matrix>")
			{
				auto const root_transform_entity_type =
					entity_manager.create_entity_type<Local_position, Local_rotation, Transform_matrix, Entity>(2, Space{ 0 });

				AND_GIVEN("Several root transform entities")
				{
					std::array<Entity, 4> root_transform_entities =
						entity_manager.create_entities<4>(root_transform_entity_type, Local_position{}, Local_rotation{}, Transform_matrix{});

					WHEN("Creating transform tree for a given root transform entity")
					{
						Entity const root_transform_entity = root_transform_entities[1];

						Transforms_tree const transform_tree =
							create_transforms_tree(entity_manager, root_transform_entity);

						THEN("The transform tree should have one entity without any children (empty transform_tree)")
						{
							CHECK(transform_tree.empty());
						}
					}

					AND_GIVEN("A Child_transform_entity_type <Local_position, Local_rotation, Transform_matrix, Transform_root, Transform_parent>")
					{
						auto const child_transform_entity_type =
							entity_manager.create_entity_type<Local_position, Local_rotation, Transform_matrix, Transform_root, Transform_parent, Entity>(2, Space{ 0 });

						AND_GIVEN("Several child transform entities")
						{
							Entity const root_transform_entity = root_transform_entities[2];

							Entity const first_child =
								entity_manager.create_entity(
									child_transform_entity_type,
									Local_position{}, Local_rotation{}, Transform_matrix{},
									Transform_root{ root_transform_entity },
									Transform_parent{ root_transform_entity }
							);

							Entity const first_child_child =
								entity_manager.create_entity(
									child_transform_entity_type,
									Local_position{}, Local_rotation{}, Transform_matrix{},
									Transform_root{ root_transform_entity },
									Transform_parent{ first_child }
							);

							Entity const second_child =
								entity_manager.create_entity(
									child_transform_entity_type,
									Local_position{}, Local_rotation{}, Transform_matrix{},
									Transform_root{ root_transform_entity },
									Transform_parent{ root_transform_entity }
							);

							Entity const second_child_child =
								entity_manager.create_entity(
									child_transform_entity_type,
									Local_position{}, Local_rotation{}, Transform_matrix{},
									Transform_root{ root_transform_entity },
									Transform_parent{ second_child }
							);

							WHEN("Creating transform tree for the root transform with childs")
							{
								Transforms_tree const transform_tree =
									create_transforms_tree(entity_manager, root_transform_entity);

								THEN("The transform tree should have the root transform with the two children")
								{
									std::deque<Entity> expected_children
									{
										first_child,
										second_child
									};

									auto const range = transform_tree.equal_range(Transform_parent{ root_transform_entity });
									REQUIRE(std::distance(range.first, range.second) == expected_children.size());

									for (auto i = range.first; i != range.second; ++i)
									{
										auto const location =
											std::find(expected_children.begin(), expected_children.end(), i->second);

										REQUIRE(location != expected_children.end());
										expected_children.erase(location);
									}

									CHECK(expected_children.empty());
								}

								THEN("The transform tree should have the first child and its respective child")
								{
									auto const range = transform_tree.equal_range(Transform_parent{ first_child });
									REQUIRE(std::distance(range.first, range.second) == 1);

									CHECK(range.first->second == first_child_child);
								}

								THEN("The transform tree should have the second child and its respective child")
								{
									auto const range = transform_tree.equal_range(Transform_parent{ second_child });
									REQUIRE(std::distance(range.first, range.second) == 1);

									CHECK(range.first->second == second_child_child);
								}
							}
						}
					}
				}
			}
		}
	}

	SCENARIO("Create a transform tree and update transforms")
	{
		GIVEN("An entity manager")
		{
			Entity_manager entity_manager{};

			AND_GIVEN("A root transform entity and 5 child transform entities")
			{
				float const pi = std::acos(-1.0f);

				auto const root_transform_entity_type = entity_manager.create_entity_type<Local_position, Local_rotation, Transform_matrix, Entity>(1, Space{ 0 });

				Entity const root_transform_entity = entity_manager.create_entity(
					root_transform_entity_type,
					Local_position{ { 1.0f, 2.0f, 3.0f } },
					Local_rotation{ { std::cos(pi / 4.0f), 0.0f, std::sin(pi / 4.0f), 0.0f } },
					Transform_matrix{}
				);


				auto const child_transform_entity_type = entity_manager.create_entity_type<Local_position, Local_rotation, Transform_matrix, Transform_root, Transform_parent, Entity>(5, Space{ 0 });

				Entity const child_transform_entity_0 = entity_manager.create_entity(
					child_transform_entity_type,
					Local_position{ { -1.0f, 0.5f, -2.0f } },
					Local_rotation{ { std::cos(pi / 4.0f), std::sin(pi / 4.0f), 0.0f, 0.0f } },
					Transform_matrix{},
					Transform_root{ root_transform_entity },
					Transform_parent{ root_transform_entity }
				);

				Entity const child_transform_entity_1 = entity_manager.create_entity(
					child_transform_entity_type,
					Local_position{ { 0.0f, 0.0f, 0.0f } },
					Local_rotation{ { std::cos(pi / 4.0f), 0.0f, 0.0f, std::sin(pi / 4.0f) } },
					Transform_matrix{},
					Transform_root{ root_transform_entity },
					Transform_parent{ child_transform_entity_0 }
				);

				Entity const child_transform_entity_2 = entity_manager.create_entity(
					child_transform_entity_type,
					Local_position{ { -0.5f, -4.0f, 5.0f } },
					Local_rotation{ { std::cos(pi / 4.0f), std::sin(pi / 4.0f), 0.0f, 0.0f } },
					Transform_matrix{},
					Transform_root{ root_transform_entity },
					Transform_parent{ child_transform_entity_1 }
				);

				Entity const child_transform_entity_3 = entity_manager.create_entity(
					child_transform_entity_type,
					Local_position{ { 0.0f, 2.0f, 5.0f } },
					Local_rotation{ { 1.0f, 0.0f, 0.0f, 0.0f } },
					Transform_matrix{},
					Transform_root{ root_transform_entity },
					Transform_parent{ child_transform_entity_0 }
				);

				Entity const child_transform_entity_4 = entity_manager.create_entity(
					child_transform_entity_type,
					Local_position{ { 0.0f, 0.0f, -3.0f } },
					Local_rotation{ { std::cos(pi / 4.0f), 0.0f, 0.0f, std::sin(pi / 4.0f) } },
					Transform_matrix{},
					Transform_root{ root_transform_entity },
					Transform_parent{ root_transform_entity }
				);

				WHEN("The root transform is created")
				{
					auto[root_position, root_rotation] = entity_manager.get_components_data<Local_position, Local_rotation>(root_transform_entity);
					Transform_matrix const root_transform_matrix = create_transform(root_position, root_rotation);
					
					THEN("The root transform is calculated correctly")
					{
						Eigen::Matrix4f expected_transform_matrix;
						expected_transform_matrix <<
							0.0f, 0.0f, 1.0f, 1.0f,
							0.0f, 1.0f, 0.0f, 2.0f,
							-1.0f, 0.0f, 0.0f, 3.0f,
							0.0f, 0.0f, 0.0f, 1.0f;

						CHECK(root_transform_matrix.value.isApprox(expected_transform_matrix));
					}

					AND_WHEN("The transform tree is created")
					{
						Transforms_tree const transform_tree =
							create_transforms_tree(entity_manager, root_transform_entity);

						AND_WHEN("The child transforms are updated")
						{
							update_child_transforms(entity_manager, transform_tree, root_transform_entity, root_transform_matrix);

							THEN("The child transform 0 is calculated correctly")
							{
								Transform_matrix const transform_matrix =
									entity_manager.get_component_data<Transform_matrix>(child_transform_entity_0);

								Eigen::Matrix4f expected_transform_matrix;
								expected_transform_matrix <<
									0.0f, 0.0f, 1.0f, 0.0f,
									1.0f, 0.0f, 0.0f, -2.5f,
									0.0f, 1.0f, 0.0f, 0.0f,
									0.0f, 0.0f, 0.0f, 1.0f;

								CHECK(transform_matrix.value.isApprox(expected_transform_matrix));
							}

							THEN("The child transform 1 is calculated correctly")
							{
								Transform_matrix const transform_matrix =
									entity_manager.get_component_data<Transform_matrix>(child_transform_entity_1);

								Eigen::Matrix4f expected_transform_matrix;
								expected_transform_matrix <<
									-1.0f, 0.0f, 0.0f, 2.5f,
									0.0f, 0.0f, 1.0f, 0.0f,
									0.0f, 1.0f, 0.0f, 0.0f,
									0.0f, 0.0f, 0.0f, 1.0f;

								CHECK(transform_matrix.value.isApprox(expected_transform_matrix));
							}

							THEN("The child transform 2 is calculated correctly")
							{
								Transform_matrix const transform_matrix =
									entity_manager.get_component_data<Transform_matrix>(child_transform_entity_2);

								Eigen::Matrix4f expected_transform_matrix;
								expected_transform_matrix <<
									-1.0f, 0.0f, 0.0f, 2.0f,
									0.0f, -1.0f, 0.0f, -4.0f,
									0.0f, 0.0f, 1.0f, 5.0f,
									0.0f, 0.0f, 0.0f, 1.0f;

								CHECK(transform_matrix.value.isApprox(expected_transform_matrix));
							}

							THEN("The child transform 3 is calculated correctly")
							{
								Transform_matrix const transform_matrix =
									entity_manager.get_component_data<Transform_matrix>(child_transform_entity_3);

								Eigen::Matrix4f expected_transform_matrix;
								expected_transform_matrix <<
									0.0f, 0.0f, 1.0f, 0.0f,
									1.0f, 0.0f, 0.0f, -0.5f,
									0.0f, 1.0f, 0.0f, 5.0f,
									0.0f, 0.0f, 0.0f, 1.0f;

								CHECK(transform_matrix.value.isApprox(expected_transform_matrix));
							}

							THEN("The child transform 4 is calculated correctly")
							{
								Transform_matrix const transform_matrix =
									entity_manager.get_component_data<Transform_matrix>(child_transform_entity_4);

								Eigen::Matrix4f expected_transform_matrix;
								expected_transform_matrix <<
									0.0f, -1.0f, 0.0f, -2.0f,
									0.0f, 0.0f, 1.0f, 1.0f,
									-1.0f, 0.0f, 0.0f, 0.0f,
									0.0f, 0.0f, 0.0f, 1.0f;

								CHECK(transform_matrix.value.isApprox(expected_transform_matrix));
							}
						}
					}
				}
			}
		}
	}

	SCENARIO("Execute transform system, check if transforms are correct and manipulate transform_dirty flags")
	{
		GIVEN("An entity manager")
		{
			Entity_manager entity_manager{};

			AND_GIVEN("A root transform entity with a dirty flag and two child transforms")
			{
				auto const root_transform_entity_type = entity_manager.create_entity_type<Local_position, Local_rotation, Transform_matrix, Transform_tree_dirty, Entity>(1, Space{ 0 });

				Entity const root_transform_entity = entity_manager.create_entity(
					root_transform_entity_type,
					Local_position{ { 1.0f, 0.0f, 0.0f } },
					Local_rotation{ { 1.0f, 0.0f, 0.0f, 0.0f } },
					Transform_matrix{},
					Transform_tree_dirty{ true }
				);


				auto const child_transform_entity_type = entity_manager.create_entity_type<Local_position, Local_rotation, Transform_matrix, Transform_root, Transform_parent, Entity>(2, Space{ 0 });

				Entity const child_transform_entity_0 = entity_manager.create_entity(
					child_transform_entity_type,
					Local_position{ { 0.0f, 2.0f, 0.0f } },
					Local_rotation{ { 1.0f, 0.0f, 0.0f, 0.0f } },
					Transform_matrix{},
					Transform_root{ root_transform_entity },
					Transform_parent{ root_transform_entity }
				);

				Entity const child_transform_entity_1 = entity_manager.create_entity(
					child_transform_entity_type,
					Local_position{ { 0.0f, 0.0f, 3.0f } },
					Local_rotation{ { 1.0f, 0.0f, 0.0f, 0.0f } },
					Transform_matrix{},
					Transform_root{ root_transform_entity },
					Transform_parent{ child_transform_entity_0 }
				);


				WHEN("The transform system is executed")
				{
					Transform_system transform_system;
					transform_system.execute(entity_manager);

					THEN("The transform_tree_dirty flag is false")
					{
						Transform_tree_dirty const transform_tree_dirty =
							entity_manager.get_component_data<Transform_tree_dirty>(root_transform_entity);

						CHECK(transform_tree_dirty.value == false);
					}

					THEN("The root transform is calculated correctly")
					{
						Transform_matrix const transform_matrix =
							entity_manager.get_component_data<Transform_matrix>(root_transform_entity);

						Eigen::Matrix4f expected_transform_matrix;
						expected_transform_matrix <<
							1.0f, 0.0f, 0.0f, 1.0f,
							0.0f, 1.0f, 0.0f, 0.0f,
							0.0f, 0.0f, 1.0f, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f;

						CHECK(transform_matrix.value.isApprox(expected_transform_matrix));
					}

					THEN("The child transform 0 is calculated correctly")
					{
						Transform_matrix const transform_matrix =
							entity_manager.get_component_data<Transform_matrix>(child_transform_entity_0);

						Eigen::Matrix4f expected_transform_matrix;
						expected_transform_matrix <<
							1.0f, 0.0f, 0.0f, 1.0f,
							0.0f, 1.0f, 0.0f, 2.0f,
							0.0f, 0.0f, 1.0f, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f;

						CHECK(transform_matrix.value.isApprox(expected_transform_matrix));
					}

					THEN("The child transform 1 is calculated correctly")
					{
						Transform_matrix const transform_matrix =
							entity_manager.get_component_data<Transform_matrix>(child_transform_entity_1);

						Eigen::Matrix4f expected_transform_matrix;
						expected_transform_matrix <<
							1.0f, 0.0f, 0.0f, 1.0f,
							0.0f, 1.0f, 0.0f, 2.0f,
							0.0f, 0.0f, 1.0f, 3.0f,
							0.0f, 0.0f, 0.0f, 1.0f;

						CHECK(transform_matrix.value.isApprox(expected_transform_matrix));
					}

					AND_WHEN("The child transform 1 position is updated, but the transform_tree_dirty flag remains false")
					{
						Transform_matrix const original_transform_matrix =
							entity_manager.get_component_data<Transform_matrix>(child_transform_entity_1);

						entity_manager.set_component_data(child_transform_entity_1, Local_position{ { 0.0f, 0.0f, 6.0f } });

						transform_system.execute(entity_manager);

						THEN("The child transform 1 is not updated")
						{
							Transform_matrix const current_transform_matrix =
								entity_manager.get_component_data<Transform_matrix>(child_transform_entity_1);

							CHECK(current_transform_matrix == original_transform_matrix);
						}

						AND_WHEN("The transform_tree_dirty flag is set to true")
						{
							entity_manager.set_component_data(root_transform_entity, Transform_tree_dirty{ true });

							transform_system.execute(entity_manager);

							THEN("The child transform 1 is updated")
							{
								Transform_matrix const transform_matrix =
									entity_manager.get_component_data<Transform_matrix>(child_transform_entity_1);

								Eigen::Matrix4f expected_transform_matrix;
								expected_transform_matrix <<
									1.0f, 0.0f, 0.0f, 1.0f,
									0.0f, 1.0f, 0.0f, 2.0f,
									0.0f, 0.0f, 1.0f, 6.0f,
									0.0f, 0.0f, 0.0f, 1.0f;

								CHECK(transform_matrix.value.isApprox(expected_transform_matrix));
							}
						}
					}
				}
			}
		}
	}
}
