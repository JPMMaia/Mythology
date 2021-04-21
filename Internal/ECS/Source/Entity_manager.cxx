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
import maia.ecs.tuple_helpers;

namespace Maia::ECS
{
    /**
     * @brief Describes the location of an entity info.
     * 
     */
    export struct Entity_info_location
    {
        /**
         * @brief Index of a vector in a tuple.
         * 
         */
        std::size_t vector_index;

        /**
         * @brief Index of the entity info inside the vector.
         * 
         */
        std::size_t index_in_vector;
    };

    /**
     * @brief Describes the location of an entity.
     * 
     */
    export template <typename Key_t>
    struct Entity_info
    {
        /**
         * @brief Index of a map in a tuple.
         * 
         */
        std::size_t map_index;

        /**
         * @brief Key in a component group map.
         * 
         */
        Key_t key;

        /**
         * @brief Index of the entity components.
         * 
         */
        std::size_t index_in_vector;
    };

    /**
     * @brief Container for entities and respective components.
     * 
     */
    export template <typename Entity_infos_t, typename Component_groups_t>
    class Entity_manager
    {
    public:

        /**
         * @brief Create entities and respective components.
         * 
         * @param number_of_entities The number of entities to create.
         * @param output_allocator The allocator used for allocating memory for the returned vector.
         * @param key The key of the component group to which the entities will be added.
         * @param components The initial values of the components associated with the entities.
         * @return The created entities.
         */
        template <typename Key_t, typename... Component_ts>
        std::pmr::vector<Entity> create_entities(
            std::size_t const number_of_entities,
            std::pmr::polymorphic_allocator<> const& output_allocator,
            Key_t&& key,
            std::tuple<Component_ts...> const& components
        )
        {
            auto& component_groups = m_component_groups;

            Key_t const key_copy = key;

            Entity_index_range const entity_index_range = 
                add_entities(
                    component_groups,
                    number_of_entities,
                    std::forward<Key_t>(key),
                    std::tuple_cat(components, std::make_tuple(Entity{0}))
                );

            using Entity_info_vector = std::pmr::vector<Entity_info<Key_t>>;

            constexpr std::size_t vector_index = Tuple_element_index<
                Entity_info_vector,
                decltype(m_entity_infos)
            >::value;

            Entity_info_vector& entity_infos = std::get<vector_index>(m_entity_infos);
            std::pmr::vector<Entity_info_location>& entity_info_locations = m_entity_info_locations;

            std::size_t const first_entity_value = entity_infos.size();
            std::size_t const last_entity_value = first_entity_value + (entity_index_range.end - entity_index_range.begin);

            {
                using Component_group_t = Component_group<Key_t, Vector_tuple<Component_ts..., Entity>>;
                Component_group_t& component_group = std::get<Component_group_t>(component_groups);
                std::pmr::vector<Entity>& entities_vector = std::get<std::pmr::vector<Entity>>(component_group.at(key_copy));

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
                        std::pmr::unordered_map<std::decay_t<Key_t>, std::tuple<std::pmr::vector<Component_ts>..., std::pmr::vector<Entity>>>,
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

        /**
         * @brief Destroy entities and respective components.
         * 
         * @param entities The entities to destroy.
         */
        void destroy_entities(
            std::span<Entity const> const entities
        ) noexcept
        {
            auto& component_groups = m_component_groups;

            std::pmr::vector<Entity_info_location> const& entity_info_locations = m_entity_info_locations;

            for (Entity const entity : entities)
            {
                Entity_info_location const& entity_info_location = entity_info_locations[entity.index];

                visit(m_entity_infos, entity_info_location.vector_index,
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
                                        Entity const entity_to_move = std::get<std::pmr::vector<Entity>>(vector_tuple).back();
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

        /**
         * @brief Get the components associated with an entity.
         * 
         * @param entity The entity that identifies the components.
         * @return The value of the entity's components.
         */
        template <typename... Component_ts>
        std::tuple<Component_ts...> get_components(
            Entity const entity
        ) const noexcept
        {
            auto const& component_groups = m_component_groups;

            std::pmr::vector<Entity_info_location> const& entity_info_locations = m_entity_info_locations;
            Entity_info_location const& entity_info_location = entity_info_locations[entity.index];

            std::tuple<Component_ts...> components{};

            visit(m_entity_infos, entity_info_location.vector_index,
                [&](auto const& entity_infos) -> void
                {
                    auto const entity_info = entity_infos[entity_info_location.index_in_vector];

                    visit(component_groups, entity_info.map_index,
                        [&] <typename T> (T const& component_map) -> void
                        {
                            if constexpr (contains<T, std::pmr::vector<Component_ts>...>())
                            {
                                auto const& vector_tuple = component_map.at(entity_info.key);

                                components = std::make_tuple(
                                    std::get<std::pmr::vector<Component_ts>>(vector_tuple)[entity_info.index_in_vector]...
                                );
                            }
                        }
                    );
                }
            );

            return components;
        }

        /**
         * @brief Set the components associated with an entity.
         * 
         * @param entity The entity that identifies the components.
         * @param components The value of the entity's components to set.
         */
        template <typename... Component_ts>
        void set_components(
            Entity const entity,
            std::tuple<Component_ts...> const& components
        ) noexcept
        {
            auto& component_groups = m_component_groups;

            std::pmr::vector<Entity_info_location> const& entity_info_locations = m_entity_info_locations;
            Entity_info_location const& entity_info_location = entity_info_locations[entity.index];

            visit(m_entity_infos, entity_info_location.vector_index,
                [&](auto const& entity_infos) -> void
                {
                    auto const entity_info = entity_infos[entity_info_location.index_in_vector];

                    visit(component_groups, entity_info.map_index,
                        [&] <typename T> (T& component_map) -> void
                        {
                            if constexpr (contains<T, std::pmr::vector<Component_ts>...>())
                            {
                                auto& vector_tuple = component_map.at(entity_info.key);

                                ((std::get<std::pmr::vector<Component_ts>>(vector_tuple)[entity_info.index_in_vector] = std::get<Component_ts>(components)), ...);
                            }
                        }
                    );
                }
            );
        }

        /**
         * @brief Get all component groups
         * 
         * @return The component groups.
         */
        Component_groups_t& get_component_groups() noexcept
        {
            return m_component_groups;
        }

        /**
         * @brief Get all component groups
         * 
         * @return The component groups.
         */
        Component_groups_t const& get_component_groups() const noexcept
        {
            return m_component_groups;
        }

    private:

        /**
         * @brief Location of each entity info.
         * 
         * Indexed by Entity::index.
         * 
         */
        std::pmr::vector<Entity_info_location> m_entity_info_locations;

        /**
         * @brief Location of each entity.
         * 
         * Indexed by Entity_info_location::vector_index.
         * 
         */
        Entity_infos_t m_entity_infos;

        /**
         * @brief The container that holds the entities' components.
         * 
         */
        Component_groups_t m_component_groups;
    };

}
