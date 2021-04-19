#include <catch2/catch.hpp>

#include <cstddef>
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
            std::vector<Entity_info<int>>
        >,
        std::tuple<
            Component_group<int, Vector_tuple<int, float, Entity>>,
            Component_group<int, Vector_tuple<int, float, double, Entity>>
        >
    >;

    TEST_CASE("create_entities adds components")
    {
        Entity_manager_0 entity_manager;

        create_entities(entity_manager, 1, {}, 0, std::make_tuple(1, 1.0f));

        {
            auto const& map = std::get<0>(entity_manager.component_groups);
            REQUIRE(map.contains(0));

            auto const& group = map.at(0);
            
            {
                std::vector<int> const& int_components = std::get<0>(group);
                REQUIRE(int_components.size() == 1);
                
                CHECK(int_components[0] == 1);
            }
            
            {
                std::vector<float> const& float_components = std::get<1>(group);
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
                create_entities(entity_manager, 1, {}, 0, std::make_tuple(1, 1.0f));

            REQUIRE(entities.size() == 1);

            {
                Entity const entity = entities[0];

                std::tuple<int, float> const components = get_components<int, float>(entity_manager, entity);
                CHECK(components == std::make_tuple(1, 1.0f));
            }
        }

        {
            std::pmr::vector<Entity> const entities =
                create_entities(entity_manager, 1, {}, 1, std::make_tuple(2, 2.0f, 2.0));

            REQUIRE(entities.size() == 1);

            {
                Entity const entity = entities[0];

                std::tuple<int, float, double> const components = get_components<int, float, double>(entity_manager, entity);
                CHECK(components == std::make_tuple(2, 2.0f, 2.0));
            }
        }

        {
            std::pmr::vector<Entity> const entities =
                create_entities(entity_manager, 1, {}, 1, std::make_tuple(3, 3.0f, 3.0));

            REQUIRE(entities.size() == 1);

            {
                Entity const entity = entities[0];

                std::tuple<int, float, double> const components = get_components<int, float, double>(entity_manager, entity);
                CHECK(components == std::make_tuple(3, 3.0f, 3.0));
            }
        }
    }

    TEST_CASE("destroy_entities removes entities and components")
    {
        Entity_manager_0 entity_manager;

        std::pmr::vector<Entity> const entities_0 =
            create_entities(entity_manager, 1, {}, 0, std::make_tuple(1, 1.0f));

        std::pmr::vector<Entity> const entities_1 =
            create_entities(entity_manager, 1, {}, 0, std::make_tuple(1, 1.0f));

        CHECK(get_number_of_entities<Vector_tuple<int, float, Entity>>(entity_manager.component_groups, 0) == 2);

        destroy_entities(entity_manager, entities_0);

        CHECK(get_number_of_entities<Vector_tuple<int, float, Entity>>(entity_manager.component_groups, 0) == 1);

        destroy_entities(entity_manager, entities_1);

        CHECK(get_number_of_entities<Vector_tuple<int, float, Entity>>(entity_manager.component_groups, 0) == 0);
    }

    TEST_CASE("destroy_entities does not invalidate other entities and components")
    {
        Entity_manager_0 entity_manager;

        std::pmr::vector<Entity> const entities_0 =
            create_entities(entity_manager, 1, {}, 0, std::make_tuple(1, 1.0f));

        std::pmr::vector<Entity> const entities_1 =
            create_entities(entity_manager, 1, {}, 0, std::make_tuple(2, 2.0f));

        destroy_entities(entity_manager, entities_0);

        REQUIRE(entities_1.size() == 1);

        {
            Entity const entity = entities_1[0];

            std::tuple<int, float> const components = get_components<int, float>(entity_manager, entity);
            CHECK(components == std::make_tuple(2, 2.0f));
        }
    }

    TEST_CASE("set_components sets the value of an entity's components")
    {
        Entity_manager_0 entity_manager;

        std::pmr::vector<Entity> const entities = create_entities(entity_manager, 1, {}, 0, std::make_tuple(0, 0.0f));
        REQUIRE(entities.size() == 1);

        {
            Entity const entity = entities[0];

            std::tuple<int, float> const new_components{1, 2.0f};
            set_components(entity_manager, entity, new_components);

            std::tuple<int, float> const actual_components = get_components<int, float>(entity_manager, entity);
            CHECK(actual_components == new_components);
        }
    }
}