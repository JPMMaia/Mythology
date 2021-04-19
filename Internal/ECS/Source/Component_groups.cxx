module;

#include <cstddef>
#include <memory_resource>
#include <tuple>
#include <unordered_map>
#include <vector>

export module maia.ecs.component_groups;

namespace Maia::ECS
{
    export template <typename... Component_ts>
    using Vector_tuple = std::tuple<std::vector<Component_ts>...>;

    export template <typename Key_t, typename Tuple>
    using Component_group = std::unordered_map<Key_t, Tuple>;

    export template <typename Tuple, typename Component_groups_t, typename Key_t>
    std::size_t get_number_of_entities(Component_groups_t const& component_groups, Key_t const& key) noexcept
    {
        using Component_group_t = Component_group<Key_t, Tuple>;

        Component_group_t const& component_group = std::get<Component_group_t>(component_groups);
        
        auto const iterator = component_group.find(key);

        return iterator != component_group.end() ?
            std::get<0>(iterator->second).size() :
            0;
    }

    export struct Entity_index_range
    {
        std::size_t begin;
        std::size_t end;
    };

    export template <typename Component_groups_t, typename Key_t, typename... Component_ts>
    Entity_index_range add_entities(
        Component_groups_t& component_groups,
        std::size_t const number_of_entities,
        Key_t&& key,
        std::tuple<Component_ts...> const& components
    )
    {
        using Component_group_t = Component_group<Key_t, Vector_tuple<Component_ts...>>;

        Component_group_t& component_group = std::get<Component_group_t>(component_groups);

        auto const add_components = [number_of_entities]<typename T>(std::vector<T>& components, T const& component)
        {
            components.insert(
                components.end(),
                number_of_entities,
                component
            );
        };

        auto& vector_tuple = component_group[key];

        std::size_t const first_entity_index = std::get<0>(vector_tuple).size();

        ((add_components(std::get<std::vector<Component_ts>>(vector_tuple), std::get<Component_ts>(components))), ...);

        std::size_t const last_entity_index = std::get<0>(vector_tuple).size();

        return {first_entity_index, last_entity_index};
    }

    export template <typename Component_groups_t, typename Function_t>
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

    export template <typename Component_group_t, typename... Component_vector_ts>
    constexpr bool contains() noexcept
    {
        return std::conjunction_v<
            Has_type<Component_vector_ts, typename Component_group_t::mapped_type>...
        >;
    }

    export template <typename Tuple, typename Function>
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
}
