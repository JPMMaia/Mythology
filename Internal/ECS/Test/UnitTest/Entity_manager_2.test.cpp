#include <catch2/catch.hpp>

#include <tuple>
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

    TEST_CASE("Add new components")
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
}