#include <catch2/catch.hpp>

#include <klein/klein.hpp>

#include <array>
#include <cmath>
#include <memory_resource>
#include <span>
#include <unordered_map>
#include <vector>

import maia.ecs.component_groups;
import maia.ecs.entity;
import maia.ecs.entity_manager;
import maia.ecs.systems.transform_system;

namespace Maia::ECS::Systems::Test
{
    TEST_CASE("compute_transform applies rotation and then translation")
    {
        float const pi = std::acos(-1.0f);
        
        kln::motor const transform = compute_transform(
                kln::translator{2.0f, 1.0f, 0.0f, 0.0f},
                kln::rotor{-0.5f * pi, 0.0f, 0.0f, 1.0f}
        );

        kln::point const point{1.0f, 0.0f, 0.0f};
        kln::point const transformed_point = transform(point).normalized();
        
        kln::point const expected_point{2.0f, 1.0f, 0.0f};
        CHECK(transformed_point.x() == expected_point.x());
        CHECK(transformed_point.y() == expected_point.y());
        CHECK(transformed_point.z() == expected_point.z());
        CHECK(transformed_point.w() == 1.0f);
    }

    TEST_CASE("get_children returns the children of an entity")
    {
        std::pmr::unordered_map<Entity, std::pmr::vector<Entity>, Entity_hash> const parent_to_children_map
        {
            std::make_pair(Entity{0}, std::pmr::vector<Entity>{Entity{1}, Entity{2}})
        };

        std::span<Entity const> const children = get_children(parent_to_children_map, Entity{0});
        REQUIRE(children.size() == 2);
        CHECK(children[0] == Entity{1});
        CHECK(children[1] == Entity{2});
    }

    TEST_CASE("get_children returns an empty span if entity is not found")
    {
        std::pmr::unordered_map<Entity, std::pmr::vector<Entity>, Entity_hash> const parent_to_children_map;

        std::span<Entity const> const children = get_children(parent_to_children_map, Entity{0});
        CHECK(children.size() == 0);
    }

    TEST_CASE("compute_world_transform applies parent transform after local transform")
    {
        float const pi = std::acos(-1.0f);
        
        kln::motor const world_transform = compute_world_transform(
            kln::motor{kln::translator{2.0f, 1.0f, 0.0f, 0.0f}},
            kln::motor{kln::rotor{-0.5f * pi, 0.0f, 0.0f, 1.0f}}
        );

        kln::point const point{1.0f, 0.0f, 0.0f};
        kln::point const transformed_point = world_transform(point).normalized();
        
        kln::point const expected_point{2.0f, 1.0f, 0.0f};
        CHECK(transformed_point.x() == expected_point.x());
        CHECK(transformed_point.y() == expected_point.y());
        CHECK(transformed_point.z() == expected_point.z());
        CHECK(transformed_point.w() == 1.0f);        
    }

    TEST_CASE("compute_world_transforms computes local transforms for root entities")
    {
        using World_motor = kln::motor;

        using Entity_manager_0 = Entity_manager<
            std::tuple<
                std::pmr::vector<Entity_info<int>>
            >,
            std::tuple<
                Component_group<int, Vector_tuple<kln::translator, kln::rotor, World_motor, Entity>>,
                Component_group<int, Vector_tuple<kln::translator, kln::rotor, World_motor, float, Entity>>
            >
        >;

        float const pi = std::acos(-1.0f);

        Entity_manager_0 entity_manager;
        
        std::array<Entity, 3> const root_entities
        {
            entity_manager.create_entities(
                1,
                {},
                0,
                std::make_tuple(
                    kln::translator{2.0f, 1.0f, 0.0f, 0.0f},
                    kln::rotor{-0.5f * pi, 0.0f, 0.0f, 1.0f},
                    World_motor{}
                )
            ).back(),
            entity_manager.create_entities(
                1,
                {},
                1,
                std::make_tuple(
                    kln::translator{3.0f, 1.0f, 0.0f, 0.0f},
                    kln::rotor{-0.5f * pi, 0.0f, 0.0f, 1.0f},
                    World_motor{}
                )
            ).back(),
            entity_manager.create_entities(
                1,
                {},
                0,
                std::make_tuple(
                    kln::translator{4.0f, 1.0f, 0.0f, 0.0f},
                    kln::rotor{-0.5f * pi, 0.0f, 0.0f, 1.0f},
                    World_motor{},
                    0.0f
                )
            ).back()
        };

        std::pmr::unordered_map<Entity, std::pmr::vector<Entity>, Entity_hash> const parent_to_children_map;

        compute_world_transforms<kln::translator, kln::rotor, kln::motor>(
            entity_manager, root_entities, parent_to_children_map
        );

        for (Entity const entity : root_entities)
        {
            std::tuple<kln::translator, kln::rotor> const translator_rotor =
                entity_manager.get_components<kln::translator, kln::rotor>(entity);

            kln::motor const expected_transform = std::get<0>(translator_rotor) * std::get<1>(translator_rotor);

            kln::motor const actual_transform =
                std::get<0>(entity_manager.get_components<kln::motor>(entity));

            CHECK(expected_transform == actual_transform);
        }
    }

    TEST_CASE("compute_world_transforms computes world transforms for non-root entities")
    {
        using World_motor = kln::motor;

        using Entity_manager_0 = Entity_manager<
            std::tuple<
                std::pmr::vector<Entity_info<int>>
            >,
            std::tuple<
                Component_group<int, Vector_tuple<int>>,
                Component_group<int, Vector_tuple<kln::translator, kln::rotor, World_motor, Entity>>
            >
        >;

        float const pi = std::acos(-1.0f);

        Entity_manager_0 entity_manager;
        
        std::pmr::vector<Entity> const entities = entity_manager.create_entities(
                4,
                {},
                0,
                std::make_tuple(
                    kln::translator{1.0f, 1.0f, 0.0f, 0.0f},
                    kln::rotor{0.0f, 1.0f, 0.0f, 0.0f},
                    World_motor{}
                )
            );

        std::array<Entity, 1> const root_entities
        {
            entities[0]
        };

        std::pmr::unordered_map<Entity, std::pmr::vector<Entity>, Entity_hash> const parent_to_children_map
        {
            std::make_pair(entities[0], std::pmr::vector<Entity>{entities[1], entities[2]}),
            std::make_pair(entities[1], std::pmr::vector<Entity>{entities[3]})
        };

        compute_world_transforms<kln::translator, kln::rotor, kln::motor>(
            entity_manager, root_entities, parent_to_children_map
        );

        {
            kln::motor const expected_transform
            {
                kln::translator{1.0f, 1.0f, 0.0f, 0.0f}
            };

            kln::motor const actual_transform =
                std::get<0>(entity_manager.get_components<kln::motor>(entities[0]));

            CHECK(expected_transform == actual_transform);            
        }

        {
            kln::motor const expected_transform
            {
                kln::translator{1.0f, 1.0f, 0.0f, 0.0f} * kln::translator{1.0f, 1.0f, 0.0f, 0.0f}
            };

            kln::motor const actual_transform =
                std::get<0>(entity_manager.get_components<kln::motor>(entities[1]));

            CHECK(expected_transform == actual_transform);            
        }

        {
            kln::motor const expected_transform
            {
                kln::translator{1.0f, 1.0f, 0.0f, 0.0f} * kln::translator{1.0f, 1.0f, 0.0f, 0.0f}
            };

            kln::motor const actual_transform =
                std::get<0>(entity_manager.get_components<kln::motor>(entities[2]));

            CHECK(expected_transform == actual_transform);            
        }

        {
            kln::motor const expected_transform
            {
                kln::translator{1.0f, 1.0f, 0.0f, 0.0f} * kln::translator{1.0f, 1.0f, 0.0f, 0.0f} * kln::translator{1.0f, 1.0f, 0.0f, 0.0f}
            };

            kln::motor const actual_transform =
                std::get<0>(entity_manager.get_components<kln::motor>(entities[3]));

            CHECK(expected_transform == actual_transform);            
        }
    }

    TEST_CASE("benchmark compute_world_transforms", "[benchmark]")
    {
        using World_motor = kln::motor;

        using Entity_manager_0 = Entity_manager<
            std::tuple<
                std::pmr::vector<Entity_info<int>>
            >,
            std::tuple<
                Component_group<int, Vector_tuple<kln::translator, kln::rotor, World_motor, Entity>>
            >
        >;

        Entity_manager_0 entity_manager;

        constexpr std::size_t number_of_entities = 4000000;
        
        std::pmr::vector<Entity> const root_entities = entity_manager.create_entities(
            number_of_entities,
            {},
            0,
            std::make_tuple(
                kln::translator{2.0f, 1.0f, 0.0f, 0.0f},
                kln::rotor{-0.5f, 0.0f, 0.0f, 1.0f},
                World_motor{}
            )
        );

        std::pmr::unordered_map<Entity, std::pmr::vector<Entity>, Entity_hash> const parent_to_children_map;

        BENCHMARK("Root entities")
        {
            compute_world_transforms<kln::translator, kln::rotor, kln::motor>(
                entity_manager, root_entities, parent_to_children_map
            );
        };
    }
}
