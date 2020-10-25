import maia.ecs.component;
import maia.ecs.component_chunk_group;
import maia.ecs.entity;

import <catch2/catch.hpp>;

import <array>;
import <cstring>;

namespace Maia::ECS::Test
{
    namespace
    {
        template<typename T>
        struct Component_base {};

        template<typename T>
        bool operator==(Component_base<T> const lhs, Component_base<T> const rhs)
        {
            return std::memcmp(&lhs, &rhs, sizeof(T));
        }

        template<typename T>
        bool operator!=(Component_base<T> const lhs, Component_base<T> const rhs)
        {
            return !(lhs == rhs);
        }

        struct Component_a : Component_base<Component_a>
        {
            int value = 1;
        };

        struct Component_b : Component_base<Component_b>
        {
            int value = 2;
        };

        struct Component_c : Component_base<Component_c>
        {
            int value = 3;
        };
    }

    TEST_CASE("Component chunk group hides the component types", "[component_chunk_group]")
    {
        std::array<Component_type_ID, 2> const ab_component_type_ids
        {
            get_component_type_id<Component_a>(),
            get_component_type_id<Component_b>()
        };

        Component_chunk_group const group{ab_component_type_ids, {}, {}};

        CHECK(group.has_component<Component_a>());
        CHECK(group.has_component<Component_b>());
        CHECK(!group.has_component<Component_c>());
    }

    TEST_CASE("Components are initialized with default values", "[component_chunk_group]")
    {
        constexpr Entity entity{ 1 };

        std::array<Component_type_ID, 2> const ab_component_type_ids
        {
            get_component_type_id<Component_a>(),
            get_component_type_id<Component_b>()
        };

        Component_chunk_group group{ab_component_type_ids, {}, {}};
        group.add_entity(entity);

        {
            constexpr Component_a expected_value{};
            Component_a const actual_value = group.get_component_value<Component_a>(entity);

            CHECK(actual_value == expected_value);
        }

        {
            constexpr Component_b expected_value{};
            Component_b const actual_value = group.get_component_value<Component_b>(entity);

            CHECK(actual_value == expected_value);
        }
    }

    TEST_CASE("Set component values", "[component_chunk_group]")
    {
        constexpr Entity entity{ 1 };

        std::array<Component_type_ID, 2> const ab_component_type_ids
        {
            get_component_type_id<Component_a>(),
            get_component_type_id<Component_b>()
        };

        Component_chunk_group group{ab_component_type_ids, {}, {}};
        group.add_entity(entity);

        {
            constexpr Component_a new_value{ .value = 10 };
            group.set_component_value(entity, new_value);

            Component_a const actual_value = group.get_component_value<Component_a>(entity);
            CHECK(actual_value == new_value);
        }

        {
            constexpr Component_b new_value{ .value = 12 };
            group.set_component_value(entity, new_value);

            Component_b const actual_value = group.get_component_value<Component_b>(entity);
            CHECK(actual_value == new_value);
        }
    }
}