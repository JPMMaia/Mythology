#include <catch2/catch.hpp>

#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace Maia::ECS::Test
{
    template <typename... Component_ts>
    using Vector_tuple = std::tuple<std::vector<Component_ts>...>;

    template <typename Key_t, typename Tuple>
    using Component_group = std::unordered_map<Key_t, Tuple>;

    using Component_groups_0 = std::tuple<
        Component_group<int, Vector_tuple<int, float>>,
        Component_group<int, Vector_tuple<int, float, double>>
    >;

    template <typename Tuple, typename Component_groups_t, typename Key_t>
    std::size_t get_number_of_entities(Component_groups_t const& component_groups, Key_t const& key) noexcept
    {
        using Component_group_t = Component_group<Key_t, Tuple>;

        Component_group_t const& component_group = std::get<Component_group_t>(component_groups);
        
        auto const iterator = component_group.find(key);

        return iterator != component_group.end() ?
            std::get<0>(iterator->second).size() :
            0;
    }

    template <typename Component_groups_t, typename Key_t, typename... Component_ts>
    void add_entities(
        Component_groups_t& component_groups,
        std::size_t const number_of_entities,
        Key_t&& key,
        Component_ts&&... components
    )
    {
        using Component_group_t = Component_group<Key_t, Vector_tuple<Component_ts...>>;

        Component_group_t& component_group = std::get<Component_group_t>(component_groups);

        auto const add_components = [number_of_entities]<typename T>(std::vector<T>& components, T&& component)
        {
            components.insert(
                components.end(),
                number_of_entities,
                std::forward<T>(component)
            );
        };

        auto& vector_tuple = component_group[key];
        ((add_components(std::get<std::vector<Component_ts>>(vector_tuple), std::forward<Component_ts>(components))), ...);
    }

    template <typename Component_groups_t, typename Function_t>
    void for_each(Component_groups_t const& component_groups, Function_t&& function) noexcept
    {
        std::apply(
            [&function] (auto const&... component_group) noexcept
            {
                ((function(component_group)), ...);
            },
            component_groups
        );
    }


    template <typename T, typename Tuple>
    struct Has_type;

    template <typename T>
    struct Has_type<T, std::tuple<>> : std::false_type {};

    template <typename T, typename U, typename... Ts>
    struct Has_type<T, std::tuple<U, Ts...>> : Has_type<T, std::tuple<Ts...>> {};

    template <typename T, typename... Ts>
    struct Has_type<T, std::tuple<T, Ts...>> : std::true_type {};

    template <typename Component_group_t, typename Component_vector>
    constexpr bool contains() noexcept
    {
        if constexpr (Has_type<Component_vector, typename Component_group_t::mapped_type>::value)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    template <typename Tuple, typename Function>
    void visit(Tuple&& tuple, std::size_t const index, Function&& function) noexcept
    {
        std::size_t counter = 0;

        auto const visit_aux = [index, &function, &counter] <typename T> (T&& value) noexcept -> void
        {
            if (index == counter)
            {
                function(value);
            }

            ++counter;
        };

        std::apply(
            [&visit_aux] (auto const&... values) noexcept
            {
                
                ((visit_aux(values)), ...);
            },
            tuple
        );
    }

    TEST_CASE("get_number_of_entities returns the number of elements in a component group")
    {
        Component_groups_0 component_groups;
        
        CHECK(get_number_of_entities<Vector_tuple<int, float>>(component_groups, 0) == 0);

        add_entities(component_groups, 1, 0, 1, 1.0f);
        CHECK(get_number_of_entities<Vector_tuple<int, float>>(component_groups, 0) == 1);

        add_entities(component_groups, 1, 0, 2, 2.0f);
        CHECK(get_number_of_entities<Vector_tuple<int, float>>(component_groups, 0) == 2);

        add_entities(component_groups, 1, 1, 3, 3.0f);
        CHECK(get_number_of_entities<Vector_tuple<int, float>>(component_groups, 1) == 1);
        CHECK(get_number_of_entities<Vector_tuple<int, float>>(component_groups, 0) == 2);

        add_entities(component_groups, 1, 0, 4, 4.0f, 4.0);
        CHECK(get_number_of_entities<Vector_tuple<int, float, double>>(component_groups, 0) == 1);
        CHECK(get_number_of_entities<Vector_tuple<int, float>>(component_groups, 1) == 1);
        CHECK(get_number_of_entities<Vector_tuple<int, float>>(component_groups, 0) == 2);

        add_entities(component_groups, 1, 1, 5, 5.0f, 5.0);
        CHECK(get_number_of_entities<Vector_tuple<int, float, double>>(component_groups, 1) == 1);
        CHECK(get_number_of_entities<Vector_tuple<int, float, double>>(component_groups, 0) == 1);
        CHECK(get_number_of_entities<Vector_tuple<int, float>>(component_groups, 1) == 1);
        CHECK(get_number_of_entities<Vector_tuple<int, float>>(component_groups, 0) == 2);
    }

    TEST_CASE("for_each iterates through all component groups")
    {
        Component_groups_0 component_groups;
        add_entities(component_groups, 1, 0, 1, 1.0f);
        add_entities(component_groups, 1, 1, 2, 2.0f);
        add_entities(component_groups, 1, 0, 3, 3.0f, 3.0);

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
            add_entities(component_groups, number_of_elements, 0, 1, 1.0f);
            add_entities(component_groups, number_of_elements, 0, 2, 2.0f, 2.0);

            BENCHMARK("New Component_group iteration")
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