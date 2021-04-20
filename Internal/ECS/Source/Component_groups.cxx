module;

#include <cstddef>
#include <memory_resource>
#include <tuple>
#include <unordered_map>
#include <vector>

export module maia.ecs.component_groups;

import maia.ecs.tuple_helpers; 

namespace Maia::ECS
{
    /**
     * @brief A component group is a map from key to a tuple of components.
     * 
     */
    export template <typename Key_t, typename Tuple>
    using Component_group = std::unordered_map<Key_t, Tuple>;

    /**
     * @brief Get the number of entities in a component group that are associated with a certain \p key.
     * 
     * @param component_groups All component groups.
     * @param key The key to use to find the number of entities.
     * @return The number of entities in a component group that are associated with a certain \p key.
     */
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

    /**
     * @brief A range of entity indices in a component group.
     * 
     */
    export struct Entity_index_range
    {
        /**
         * @brief First entity index in the range.
         * 
         */
        std::size_t begin;

        /**
         * @brief One past the last entity index in the range.
         * 
         */
        std::size_t end;
    };

    /**
     * @brief Add entities and corresponding components.
     * 
     * @param component_groups All component groups.
     * @param number_of_entities The number of entities to add. The components of each entity will be identical.
     * @param key The key on the component group to add the entities.
     * @param components The initial value of the added entities' components.
     * @return A range of entity indices describing the position in which these were added.
     */
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

    /**
     * @brief Iterate through all component groups.
     * 
     * @param component_groups Component groups to iterate.
     * @param function Function that will be called for each component group.
     */
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

    /**
     * @brief Check if component group contains the given component vectors.
     * 
     * @return True if component group contains all component vectors. False otherwise.
     */
    export template <typename Component_group_t, typename... Component_vector_ts>
    constexpr bool contains() noexcept
    {
        return std::conjunction_v<
            Has_type<Component_vector_ts, typename Component_group_t::mapped_type>...
        >;
    }

    /**
     * @brief Call function with the tuple element corresponding to the given non-compile-time index.
     * 
     * @param tuple Tuple which contains element to visit.
     * @param index Index of the tuple element to visit.
     * @param function Function that is called with the tuple element that corresponds to the given index.
     */
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
