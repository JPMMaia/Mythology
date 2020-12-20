import maia.ecs.component;
import maia.ecs.component_chunk;

import <catch2/catch.hpp>;

import <array>;

namespace Maia::ECS::Test
{
    namespace
    {
        struct Component_a
        {
            int value = 1;
        };

        struct Component_b
        {
            int value = 2;
        };

        struct Component_c
        {
            int value = 3;
        };

        template<typename T>
        concept Test_component = 
            requires (T x)
            {
                {x.value};
            };

        template <Test_component Component_t>
        bool operator==(Component_t const lhs, Component_t const rhs) noexcept
        {
            return lhs.value == rhs.value;
        }

        template <Test_component Component_t>
        bool operator!=(Component_t const lhs, Component_t const rhs) noexcept
        {
            return !(lhs == rhs);
        }
    }

    TEST_CASE("A component chunk is a container for different component types", "[Component_chunk]")
    {
        std::array<Component_type_info, 2> const component_type_infos = 
            make_sorted_component_type_info_array<Component_a, Component_b>();

        Component_chunk const component_chunk{component_type_infos, 1, {}};

        CHECK(component_chunk.has_component_type(component_type_infos[0].id));
        CHECK(component_chunk.has_component_type(component_type_infos[1].id));
        CHECK(!component_chunk.has_component_type(get_component_type_id<Component_c>()));
    }

    TEST_CASE("A component chunk is a fixed sized container", "[Component_chunk]")
    {
        std::array<Component_type_info, 2> const component_type_infos = 
            make_sorted_component_type_info_array<Component_a, Component_b>();

        Component_chunk const component_chunk{component_type_infos, 3, {}};

        CHECK(component_chunk.size() == 3);
    }

    TEST_CASE("Set components in a chunk", "[Component_chunk]")
    {
        std::array<Component_type_info, 2> const component_type_infos = 
            make_sorted_component_type_info_array<Component_a, Component_b>();

        Component_chunk component_chunk{component_type_infos, 2, {}};

        {
            constexpr Component_chunk::Size_type index = 0;
            constexpr Component_a value{.value=5};
            component_chunk.set_component_value(index, value);
            
            CHECK(component_chunk.get_component_value<Component_a>(index) == value);
        }

        {
            constexpr Component_chunk::Size_type index = 1;
            constexpr Component_a value{.value=10};
            component_chunk.set_component_value(index, value);
            
            CHECK(component_chunk.get_component_value<Component_a>(index) == value);
        }

        {
            constexpr Component_chunk::Size_type index = 0;
            constexpr Component_b value{.value=12};
            component_chunk.set_component_value(index, value);
            
            CHECK(component_chunk.get_component_value<Component_b>(index) == value);
        }

        {
            constexpr Component_chunk::Size_type index = 1;
            constexpr Component_b value{.value=15};
            component_chunk.set_component_value(index, value);
            
            CHECK(component_chunk.get_component_value<Component_b>(index) == value);
        }

        {
            constexpr Component_chunk::Size_type index = 0;
            constexpr Component_a value{.value=6};
            component_chunk.set_component_value(index, value);
            
            CHECK(component_chunk.get_component_value<Component_a>(index) == value);
        }
    }
}