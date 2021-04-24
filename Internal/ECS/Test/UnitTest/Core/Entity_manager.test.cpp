#include <catch2/catch.hpp>

#include <cstddef>
#include <memory_resource>
#include <ostream>
#include <span>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

import maia.ecs.component_groups;
import maia.ecs.entity;
import maia.ecs.entity_manager;
import maia.test.print_tuple;

namespace Maia::ECS::Test
{
    using Entity_manager_0 = Entity_manager<
        std::tuple<
            std::pmr::vector<Entity_info<int>>
        >,
        std::tuple<
            Component_group<int, Vector_tuple<int, float, Entity>>,
            Component_group<int, Vector_tuple<int, float, double, Entity>>
        >
    >;

    TEST_CASE("create_entities adds components")
    {
        Entity_manager_0 entity_manager;

        entity_manager.create_entities(1, {}, 0, std::make_tuple(1, 1.0f));

        {
            auto const& map = std::get<0>(entity_manager.get_component_groups());
            REQUIRE(map.contains(0));

            auto const& group = map.at(0);
            
            {
                std::pmr::vector<int> const& int_components = std::get<0>(group);
                REQUIRE(int_components.size() == 1);
                
                CHECK(int_components[0] == 1);
            }
            
            {
                std::pmr::vector<float> const& float_components = std::get<1>(group);
                REQUIRE(float_components.size() == 1);
                
                CHECK(float_components[0] == 1.0f);
            }
        }
    }

    TEST_CASE("create_entities returns objects that identify components")
    {
        Entity_manager_0 entity_manager;

        {
            std::pmr::vector<Entity> const entities =
                entity_manager.create_entities(1, {}, 0, std::make_tuple(1, 1.0f));

            REQUIRE(entities.size() == 1);

            {
                Entity const entity = entities[0];

                std::tuple<int, float> const components =
                    entity_manager.get_components<int, float>(entity);
                CHECK(components == std::make_tuple(1, 1.0f));
            }
        }

        {
            std::pmr::vector<Entity> const entities =
                entity_manager.create_entities(1, {}, 1, std::make_tuple(2, 2.0f, 2.0));

            REQUIRE(entities.size() == 1);

            {
                Entity const entity = entities[0];

                std::tuple<int, float, double> const components =
                    entity_manager.get_components<int, float, double>(entity);
                CHECK(components == std::make_tuple(2, 2.0f, 2.0));
            }
        }

        {
            std::pmr::vector<Entity> const entities =
                entity_manager.create_entities(1, {}, 1, std::make_tuple(3, 3.0f, 3.0));

            REQUIRE(entities.size() == 1);

            {
                Entity const entity = entities[0];

                std::tuple<int, float, double> const components = 
                    entity_manager.get_components<int, float, double>(entity);
                CHECK(components == std::make_tuple(3, 3.0f, 3.0));
            }
        }
    }

    TEST_CASE("destroy_entities removes entities and components")
    {
        Entity_manager_0 entity_manager;

        std::pmr::vector<Entity> const entities_0 =
            entity_manager.create_entities(1, {}, 0, std::make_tuple(1, 1.0f));

        std::pmr::vector<Entity> const entities_1 =
            entity_manager.create_entities(1, {}, 0, std::make_tuple(1, 1.0f));

        CHECK(get_number_of_entities<Vector_tuple<int, float, Entity>>(entity_manager.get_component_groups(), 0) == 2);

        entity_manager.destroy_entities(entities_0);

        CHECK(get_number_of_entities<Vector_tuple<int, float, Entity>>(entity_manager.get_component_groups(), 0) == 1);

        entity_manager.destroy_entities(entities_1);

        CHECK(get_number_of_entities<Vector_tuple<int, float, Entity>>(entity_manager.get_component_groups(), 0) == 0);
    }

    TEST_CASE("destroy_entities does not invalidate other entities and components")
    {
        Entity_manager_0 entity_manager;

        std::pmr::vector<Entity> const entities_0 =
            entity_manager.create_entities(1, {}, 0, std::make_tuple(1, 1.0f));

        std::pmr::vector<Entity> const entities_1 =
            entity_manager.create_entities(1, {}, 0, std::make_tuple(2, 2.0f));

        entity_manager.destroy_entities(entities_0);

        REQUIRE(entities_1.size() == 1);

        {
            Entity const entity = entities_1[0];

            std::tuple<int, float> const components = entity_manager.get_components<int, float>(entity);
            CHECK(components == std::make_tuple(2, 2.0f));
        }
    }

    TEST_CASE("set_components sets the value of an entity's components")
    {
        Entity_manager_0 entity_manager;

        std::pmr::vector<Entity> const entities = entity_manager.create_entities(1, {}, 0, std::make_tuple(0, 0.0f));
        REQUIRE(entities.size() == 1);

        {
            Entity const entity = entities[0];

            std::tuple<int, float> const new_components{1, 2.0f};
            entity_manager.set_components(entity, new_components);

            std::tuple<int, float> const actual_components = entity_manager.get_components<int, float>(entity);
            CHECK(actual_components == new_components);
        }
    }

    TEST_CASE("Create entity manager with custom allocator")
    {
        std::pmr::monotonic_buffer_resource buffer_resource;
        std::pmr::polymorphic_allocator<> allocator{&buffer_resource};
        Entity_manager_0 entity_manager{allocator};

        entity_manager.create_entities(1, {}, 0, std::make_tuple(0, 0.0f));

        for_each(
            entity_manager.get_component_groups(),
            [&](auto const& component_group) noexcept
            {
                CHECK(component_group.get_allocator() == allocator);

                for (auto const& pair : component_group)
                {
                    for_each(
                        pair.second,
                        [&](auto const& components_vector) noexcept
                        {
                            CHECK(components_vector.get_allocator() == allocator);
                        }
                    );
                }
            }
        );
    }
}
