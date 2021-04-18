#include <catch2/catch.hpp>

#include <iostream>
#include <ostream>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace Catch
{
    template<typename... Component_ts>
    struct StringMaker<std::tuple<Component_ts...>>
    {
        static std::string convert(std::tuple<Component_ts...> const& components)
        {
            using Tuple_t = std::tuple<Component_ts...>;
            using Last_component_t = std::tuple_element_t<std::tuple_size_v<Tuple_t> - 1, Tuple_t>; 

            std::stringstream stream;
            stream << '{';
            ((stream << std::get<Component_ts>(components) << (std::is_same_v<Component_ts, Last_component_t> ? "" : ", ")), ...);
            stream << '}';

            return stream.str();
        }
    };
}

namespace Maia::ECS::Test
{
    template <class T, class Tuple>
    struct Tuple_element_index;

    template <class T, class... Types>
    struct Tuple_element_index<T, std::tuple<T, Types...>>
    {
        static constexpr std::size_t value = 0;
    };

    template <class T, class U, class... Types>
    struct Tuple_element_index<T, std::tuple<U, Types...>>
    {
        static constexpr std::size_t value = 1 + Tuple_element_index<T, std::tuple<Types...>>::value;
    };

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

    struct Entity_index_range
    {
        std::size_t begin;
        std::size_t end;
    };

    template <typename Component_groups_t, typename Key_t, typename... Component_ts>
    Entity_index_range add_entities(
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

        std::size_t const first_entity_index = std::get<0>(vector_tuple).size();

        ((add_components(std::get<std::vector<Component_ts>>(vector_tuple), std::forward<Component_ts>(components))), ...);

        std::size_t const last_entity_index = std::get<0>(vector_tuple).size();

        return {first_entity_index, last_entity_index};
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

    template <typename Component_group_t, typename... Component_vector_ts>
    constexpr bool contains() noexcept
    {
        return std::conjunction_v<
            Has_type<Component_vector_ts, typename Component_group_t::mapped_type>...
        >;
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
            [&visit_aux] <typename... T> (T&&... values) noexcept
            {
                ((visit_aux(std::forward<T>(values))), ...);
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

    struct Entity_info_location
    {
        std::size_t vector_index;
        std::size_t index_in_vector;
    };

    template <typename Key_t>
    struct Entity_info
    {
        std::size_t map_index;
        Key_t key;
        std::size_t index_in_vector;
    };

    template <typename... Key_ts>
    struct Entity_manager
    {
        std::vector<Entity_info_location> entity_info_locations;
        
        std::tuple<
            std::vector<Entity_info<Key_ts>>...
        > entity_infos;
    };

    using Entity_manager_0 = Entity_manager<int>;
    struct Entity
    {
        std::size_t index;
    };

    template <typename Entity_manager_t, typename Component_groups_t, typename Key_t, typename... Component_ts>
    std::pmr::vector<Entity> create_entities(
        Entity_manager_t& entity_manager,
        Component_groups_t& component_groups,
        std::size_t const number_of_entities,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        Key_t&& key,
        Component_ts&&... components
    )
    {
        Key_t const key_copy = key;

        Entity_index_range const entity_index_range = 
            add_entities(
                component_groups,
                number_of_entities,
                std::forward<Key_t>(key),
                std::forward<Component_ts>(components)...
            );

        using Entity_info_vector = std::vector<Entity_info<Key_t>>;
            
        constexpr std::size_t vector_index = Tuple_element_index<
            Entity_info_vector,
            decltype(entity_manager.entity_infos)
        >::value;

        Entity_info_vector& entity_infos = std::get<vector_index>(entity_manager.entity_infos);
        std::vector<Entity_info_location>& entity_info_locations = entity_manager.entity_info_locations;

        std::size_t const first_entity_value = entity_infos.size();
        std::size_t const last_entity_value = first_entity_value + (entity_index_range.end - entity_index_range.begin);

        for (std::size_t entity_index = entity_index_range.begin; entity_index < entity_index_range.end; ++entity_index)
        {
            {
                Entity_info_location const entity_info_location
                {
                    .vector_index = vector_index,
                    .index_in_vector = entity_infos.size(),
                };

                entity_info_locations.push_back(entity_info_location);
            }

            {
                constexpr std::size_t map_index = Tuple_element_index<
                    std::unordered_map<std::decay_t<Key_t>, std::tuple<std::vector<Component_ts>...>>,
                    Component_groups_t
                >::value;

                Entity_info<Key_t> const entity_info
                {
                    .map_index = map_index,
                    .key = key_copy,
                    .index_in_vector = entity_index,
                };

                entity_infos.push_back(entity_info);
            }
        }

        std::pmr::vector<Entity> new_entities{output_allocator};
        new_entities.resize(last_entity_value - first_entity_value);
        for (std::size_t index = 0; index < new_entities.size(); ++index)
        {
            new_entities[index] = {first_entity_value + index};
        }

        return new_entities;
    }

    template <typename... Component_ts, typename Entity_manager_t, typename Component_groups_t>
    std::tuple<Component_ts...> get_components(
        Entity_manager_t const& entity_manager,
        Component_groups_t const& component_groups,
        Entity const entity
    ) noexcept
    {
        std::vector<Entity_info_location> const& entity_info_locations = entity_manager.entity_info_locations;
        Entity_info_location const& entity_info_location = entity_info_locations[entity.index];

        std::tuple<Component_ts...> components{};

        visit(entity_manager.entity_infos, entity_info_location.vector_index,
            [&](auto const& entity_infos) -> void
            {
                auto const entity_info = entity_infos[entity_info_location.index_in_vector];

                visit(component_groups, entity_info.map_index,
                    [&] <typename T> (T const& component_map) -> void
                    {
                        if constexpr (contains<T, std::vector<Component_ts>...>())
                        {
                            auto const& vector_tuple = component_map.at(entity_info.key);

                            components = std::make_tuple(
                                std::get<std::vector<Component_ts>>(vector_tuple)[entity_info.index_in_vector]...
                            );
                        }
                    }
                );
            }
        );

        return components;
    }

    template <typename... Component_ts, typename Entity_manager_t, typename Component_groups_t>
    void set_components(
        Entity_manager_t const& entity_manager,
        Component_groups_t& component_groups,
        Entity const entity,
        std::tuple<Component_ts...> const& components
    ) noexcept
    {
        std::vector<Entity_info_location> const& entity_info_locations = entity_manager.entity_info_locations;
        Entity_info_location const& entity_info_location = entity_info_locations[entity.index];

        visit(entity_manager.entity_infos, entity_info_location.vector_index,
            [&](auto const& entity_infos) -> void
            {
                auto const entity_info = entity_infos[entity_info_location.index_in_vector];

                visit(component_groups, entity_info.map_index,
                    [&] <typename T> (T& component_map) -> void
                    {
                        if constexpr (contains<T, std::vector<Component_ts>...>())
                        {
                            auto& vector_tuple = component_map.at(entity_info.key);

                            ((std::get<std::vector<Component_ts>>(vector_tuple)[entity_info.index_in_vector] = std::get<Component_ts>(components)), ...);
                        }
                    }
                );
            }
        );
    }

    TEST_CASE("create_entities adds components")
    {
        Entity_manager_0 entity_manager;
        Component_groups_0 component_groups;

        create_entities(entity_manager, component_groups, 1, {}, 0, 1, 1.0f);

        {
            auto const& map = std::get<0>(component_groups);
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
        Component_groups_0 component_groups;

        {
            std::pmr::vector<Entity> const entities =
                create_entities(entity_manager, component_groups, 1, {}, 0, 1, 1.0f);

            REQUIRE(entities.size() == 1);

            {
                Entity const entity = entities[0];

                std::tuple<int, float> const components = get_components<int, float>(entity_manager, component_groups, entity);
                CHECK(components == std::make_tuple(1, 1.0f));
            }
        }

        {
            std::pmr::vector<Entity> const entities =
                create_entities(entity_manager, component_groups, 1, {}, 1, 2, 2.0f, 2.0);

            REQUIRE(entities.size() == 1);

            {
                Entity const entity = entities[0];

                std::tuple<int, float, double> const components = get_components<int, float, double>(entity_manager, component_groups, entity);
                CHECK(components == std::make_tuple(2, 2.0f, 2.0));
            }
        }

        {
            std::pmr::vector<Entity> const entities =
                create_entities(entity_manager, component_groups, 1, {}, 1, 3, 3.0f, 3.0);

            REQUIRE(entities.size() == 1);

            {
                Entity const entity = entities[0];

                std::tuple<int, float, double> const components = get_components<int, float, double>(entity_manager, component_groups, entity);
                CHECK(components == std::make_tuple(3, 3.0f, 3.0));
            }
        }
    }

    TEST_CASE("set_components sets the value of an entity's components")
    {
        Entity_manager_0 entity_manager;
        Component_groups_0 component_groups;

        std::pmr::vector<Entity> const entities = create_entities(entity_manager, component_groups, 1, {}, 0, 0, 0.0f);
        REQUIRE(entities.size() == 1);

        {
            Entity const entity = entities[0];

            std::tuple<int, float> const new_components{1, 2.0f};
            set_components(entity_manager, component_groups, entity, new_components);

            std::tuple<int, float> const actual_components = get_components<int, float>(entity_manager, component_groups, entity);
            CHECK(actual_components == new_components);
        }
    }
}