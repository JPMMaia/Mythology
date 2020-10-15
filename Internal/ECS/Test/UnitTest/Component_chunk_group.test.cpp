import maia.ecs.component;
import maia.ecs.component_chunk_group;
import maia.ecs.entity;

import <catch2/catch.hpp>;

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

    TEST_CASE("Use component chunk group")
    {
        SECTION("Component chunk group hides the component types")
        {
            Component_chunk_group group = create_component_chunk_group<Component_a, Component_b>();

            CHECK(group.has_component<Component_a>());
            CHECK(group.has_component<Component_b>());
            CHECK(!group.has_component<Component_c>());
        }

        SECTION("Components are initialized with default values")
        {
            Component_chunk_group group = create_component_chunk_group<Component_a, Component_b>();
            group.add_entity(Entity{1});

            {
                constexpr Component_a expected_value{};
                Component_a const actual_value = group.get_component_value<Component_a>();

                CHECK(actual_value == expected_value);
            }

            {
                constexpr Component_b expected_value{};
                Component_b const actual_value = group.get_component_value<Component_b>();

                CHECK(actual_value == expected_value);
            }
        }
    }
}