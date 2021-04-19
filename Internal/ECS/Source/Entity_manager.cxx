module;

#include <cstddef>
#include <memory_resource>
#include <span>
#include <tuple>
#include <unordered_map>
#include <vector>

export module maia.ecs.entity_manager;

import maia.ecs.component_groups;
import maia.ecs.entity;

namespace Maia::ECS
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

    export struct Entity_info_location
    {
        std::size_t vector_index;
        std::size_t index_in_vector;
    };

    export template <typename Key_t>
    struct Entity_info
    {
        std::size_t map_index;
        Key_t key;
        std::size_t index_in_vector;
    };

    export template <typename Entity_infos_t, typename Component_groups_t>
    struct Entity_manager
    {
        std::vector<Entity_info_location> entity_info_locations;
        Entity_infos_t entity_infos;
        Component_groups_t component_groups;
    };

    export template <typename Entity_manager_t, typename Key_t, typename... Component_ts>
    std::pmr::vector<Entity> create_entities(
        Entity_manager_t& entity_manager,
        std::size_t const number_of_entities,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        Key_t&& key,
        std::tuple<Component_ts...> const& components
    )
    {
        using Component_groups_t = decltype(entity_manager.component_groups);
        auto& component_groups = entity_manager.component_groups;

        Key_t const key_copy = key;

        Entity_index_range const entity_index_range = 
            add_entities(
                component_groups,
                number_of_entities,
                std::forward<Key_t>(key),
                std::tuple_cat(components, std::make_tuple(Entity{0}))
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

        {
            using Component_group_t = Component_group<Key_t, Vector_tuple<Component_ts..., Entity>>;
            Component_group_t& component_group = std::get<Component_group_t>(component_groups);
            std::vector<Entity>& entities_vector = std::get<std::vector<Entity>>(component_group.at(key_copy));

            for (std::size_t entity_index = entity_index_range.begin; entity_index < entity_index_range.end; ++entity_index)
            {
                entities_vector[entity_index] = {first_entity_value + (entity_index - entity_index_range.begin)};
            }
        }

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
                    std::unordered_map<std::decay_t<Key_t>, std::tuple<std::vector<Component_ts>..., std::vector<Entity>>>,
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

    export template <typename Entity_manager_t>
    void destroy_entities(
        Entity_manager_t& entity_manager,
        std::span<Entity const> const entities
    ) noexcept
    {
        auto& component_groups = entity_manager.component_groups;

        std::vector<Entity_info_location> const& entity_info_locations = entity_manager.entity_info_locations;

        for (Entity const entity : entities)
        {
            Entity_info_location const& entity_info_location = entity_info_locations[entity.index];

            visit(entity_manager.entity_infos, entity_info_location.vector_index,
                [&](auto& entity_infos) -> void
                {
                    auto const entity_info = entity_infos[entity_info_location.index_in_vector];

                    visit(component_groups, entity_info.map_index,
                        [&] <typename T> (T& component_map) -> void
                        {
                            auto& vector_tuple = component_map.at(entity_info.key);
                            
                            if ((entity_info.index_in_vector + 1) == std::get<0>(vector_tuple).size())
                            {
                                std::apply(
                                    [](auto&... vector) -> void
                                    {
                                        ((vector.pop_back()), ...);
                                    },
                                    vector_tuple
                                );
                            }
                            else
                            {
                                std::size_t const entity_to_remove_index = entity_info.index_in_vector;

                                {
                                    Entity const entity_to_move = std::get<std::vector<Entity>>(vector_tuple).back();
                                    Entity_info_location const& entity_to_move_info_location = entity_info_locations[entity_to_move.index];
                                    auto& entity_to_move_info = entity_infos[entity_to_move_info_location.index_in_vector];
                                    entity_to_move_info.index_in_vector = entity_to_remove_index;
                                }

                                std::apply(
                                    [entity_to_remove_index] (auto&... vector) -> void
                                    {
                                        ((vector[entity_to_remove_index] = vector.back()), ...);
                                        ((vector.pop_back()), ...);
                                    },
                                    vector_tuple
                                );
                            }
                        }
                    );
                }
            );
        }
    }

    export template <typename... Component_ts, typename Entity_manager_t>
    std::tuple<Component_ts...> get_components(
        Entity_manager_t const& entity_manager,
        Entity const entity
    ) noexcept
    {
        auto const& component_groups = entity_manager.component_groups;

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

    export template <typename... Component_ts, typename Entity_manager_t>
    void set_components(
        Entity_manager_t& entity_manager,
        Entity const entity,
        std::tuple<Component_ts...> const& components
    ) noexcept
    {
        auto& component_groups = entity_manager.component_groups;

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
}
