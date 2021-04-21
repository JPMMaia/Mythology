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
import maia.test.print_tuple;

namespace Maia::ECS::Test
{
    using Component_groups_0 = std::tuple<
        Component_group<int, Vector_tuple<int, float>>,
        Component_group<int, Vector_tuple<int, float, double>>
    >;

    TEST_CASE("get_number_of_entities returns the number of elements in a component group")
    {
        Component_groups_0 component_groups;
        
        CHECK(get_number_of_entities<Vector_tuple<int, float>>(component_groups, 0) == 0);

        add_entities(component_groups, 1, 0, std::make_tuple(1, 1.0f));
        CHECK(get_number_of_entities<Vector_tuple<int, float>>(component_groups, 0) == 1);

        add_entities(component_groups, 1, 0, std::make_tuple(2, 2.0f));
        CHECK(get_number_of_entities<Vector_tuple<int, float>>(component_groups, 0) == 2);

        add_entities(component_groups, 1, 1, std::make_tuple(3, 3.0f));
        CHECK(get_number_of_entities<Vector_tuple<int, float>>(component_groups, 1) == 1);
        CHECK(get_number_of_entities<Vector_tuple<int, float>>(component_groups, 0) == 2);

        add_entities(component_groups, 1, 0, std::make_tuple(4, 4.0f, 4.0));
        CHECK(get_number_of_entities<Vector_tuple<int, float, double>>(component_groups, 0) == 1);
        CHECK(get_number_of_entities<Vector_tuple<int, float>>(component_groups, 1) == 1);
        CHECK(get_number_of_entities<Vector_tuple<int, float>>(component_groups, 0) == 2);

        add_entities(component_groups, 1, 1, std::make_tuple(5, 5.0f, 5.0));
        CHECK(get_number_of_entities<Vector_tuple<int, float, double>>(component_groups, 1) == 1);
        CHECK(get_number_of_entities<Vector_tuple<int, float, double>>(component_groups, 0) == 1);
        CHECK(get_number_of_entities<Vector_tuple<int, float>>(component_groups, 1) == 1);
        CHECK(get_number_of_entities<Vector_tuple<int, float>>(component_groups, 0) == 2);
    }

    TEST_CASE("for_each iterates through all component groups")
    {
        Component_groups_0 component_groups;
        add_entities(component_groups, 1, 0, std::make_tuple(1, 1.0f));
        add_entities(component_groups, 1, 1, std::make_tuple(2, 2.0f));
        add_entities(component_groups, 1, 0, std::make_tuple(3, 3.0f, 3.0));

        bool int_float_iterated = false;
        bool int_float_double_iterated = false;

        for_each(
            component_groups,
            [&]<typename T>(T const& component_group) noexcept
            {
                if constexpr (std::is_same_v<T, Component_group<int, Vector_tuple<int, float>>>)
                {
                    CHECK(int_float_iterated == false);
                    int_float_iterated = true;
                }
                else if (std::is_same_v<T, Component_group<int, Vector_tuple<int, float, double>>>)
                {
                    CHECK(int_float_double_iterated == false);
                    int_float_double_iterated = true;
                }
                else
                {
                    FAIL("Component group type not detected!");
                }
            }
        );

        CHECK(int_float_iterated);
        CHECK(int_float_double_iterated);
    }

    TEST_CASE("contains returns true if tuple contains type and false otherwise")
    {
        CHECK(contains<std::tuple_element_t<0, Component_groups_0>, std::vector<int>>());
        CHECK(contains<std::tuple_element_t<0, Component_groups_0>, std::vector<float>>());
        CHECK(!contains<std::tuple_element_t<0, Component_groups_0>, std::vector<double>>());
        CHECK(contains<std::tuple_element_t<1, Component_groups_0>, std::vector<int>>());
        CHECK(contains<std::tuple_element_t<1, Component_groups_0>, std::vector<float>>());
        CHECK(contains<std::tuple_element_t<1, Component_groups_0>, std::vector<double>>());
    }

    TEST_CASE("visit calls function with tuple argument at runtime index")
    {
        std::tuple<int, float, double> tuple;

        {
            std::size_t count = 0;

            auto const visit_element = [&count]<typename T>(T const& value) -> void
            {
                if constexpr (std::is_same_v<T, int>)
                {
                    ++count;
                }
                else
                {
                    FAIL_CHECK();
                }
            };

            visit(tuple, 0, visit_element);
            CHECK(count == 1);
        }

        {
            std::size_t count = 0;

            auto const visit_element = [&count]<typename T>(T const& value) -> void
            {
                if constexpr (std::is_same_v<T, float>)
                {
                    ++count;
                }
                else
                {
                    FAIL_CHECK();
                }
            };

            visit(tuple, 1, visit_element);
            CHECK(count == 1);
        }

        {
            std::size_t count = 0;

            auto const visit_element = [&count]<typename T>(T const& value) -> void
            {
                if constexpr (std::is_same_v<T, double>)
                {
                    ++count;
                }
                else
                {
                    FAIL_CHECK();
                }
            };

            visit(tuple, 2, visit_element);
            CHECK(count == 1);
        }
    }

    TEST_CASE("Benchmark iterate through components", "[benchmark]")
    {
        constexpr std::size_t number_of_elements = 500000;

        {
            std::vector<int> int_components_0;
            int_components_0.resize(number_of_elements, 1);

            std::vector<int> int_components_1;
            int_components_1.resize(number_of_elements, 2);

            BENCHMARK("Vector iteration")
            {
                int dummy = 0;

                for (int const value : int_components_0)
                {
                    dummy += value;
                }

                for (int const value : int_components_1)
                {
                    dummy += value;
                }

                return dummy;
            };
        }

        {
            Component_groups_0 component_groups;
            add_entities(component_groups, number_of_elements, 0, std::make_tuple(1, 1.0f));
            add_entities(component_groups, number_of_elements, 0, std::make_tuple(2, 2.0f, 2.0));

            BENCHMARK("Component_group iteration")
            {
                int dummy = 0;

                for_each(
                    component_groups,
                    [&dummy]<typename T>(T const& component_group) -> void
                    {
                        if constexpr (contains<T, std::vector<int>>())
                        {
                            int dummy_2 = 0;

                            for (auto const& pair : component_group)
                            {
                                std::vector<int> const& int_components = std::get<std::vector<int>>(pair.second);

                                for (int const value : int_components)
                                {
                                    dummy_2 += value;
                                }
                            }

                            dummy += dummy_2;
                        }
                    }
                );

                return dummy;
            };
        }
    }
}