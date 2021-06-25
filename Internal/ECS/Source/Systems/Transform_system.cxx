module;

#include <memory_resource>
#include <span>
#include <unordered_map>
#include <thread>
#include <vector>

export module maia.ecs.systems.transform_system;

import maia.ecs.entity;

namespace Maia::ECS::Systems
{
    /**
     * @brief Compute transform that represents a rotation followed by a translation.
     * 
     * @param translator Translation component.
     * @param rotor Rotation component.
     * @return Transform that results from first rotating and then translating.
     */
    export template <typename Translator_t, typename Rotor_t>
    auto compute_transform(Translator_t const& translator, Rotor_t const& rotor) noexcept
    {
        return translator * rotor;
    }

    /**
     * @brief Get an entity's children.
     * 
     * @param parent_to_children_map Map from entity to its children.
     * @param entity Entity to get the children.
     * @return A span of the entity's children. Can be empty if entity hasn't any children.
     */
    export std::span<Entity const> get_children(
        std::pmr::unordered_map<Entity, std::pmr::vector<Entity>, Entity_hash> const& parent_to_children_map,
        Entity const entity
    ) noexcept
    {
        auto const iterator = parent_to_children_map.find(entity);

        if (iterator != parent_to_children_map.end())
        {
            return iterator->second;
        }
        else
        {
            return {};
        }
    }

    /**
     * @brief Compute the transform by applying the local transform and then the parent transform.
     * 
     * @param parent_transform The parent's transform.
     * @param local_transform The local transform.
     * @return The combined transform that results from applying the local transform and then the parent transform.
     */
    export template <typename Motor_t>
    Motor_t compute_world_transform(
        Motor_t const& parent_transform,
        Motor_t const& local_transform
    ) noexcept
    {
        return parent_transform * local_transform;
    }

    /**
     * @brief Compute the world transforms for all children of \p parent_entity.
     * 
     * @param entity_manager The container for all entitites and its components.
     * @param parent_entity The entity whose children the transforms will be calculated.
     * @param parent_transform The transform of \p parent_entity.
     * @param parent_to_children_map Map from entity to its children.
     */
    template <typename Entity_manager_t, typename Motor_t>
    void compute_world_transforms_for_children(
        Entity_manager_t& entity_manager,
        Entity const parent_entity,
        Motor_t const& parent_transform,
        std::pmr::unordered_map<Entity, std::pmr::vector<Entity>, Entity_hash> const& parent_to_children_map
    ) noexcept
    {
        std::span<Entity const> const child_entities =
            get_children(parent_to_children_map, parent_entity);

        // Can be threaded
        for (Entity const child_entity : child_entities)
        {
            Motor_t* const child_transform =
                std::get<0>(entity_manager.get_component_pointers<Motor_t>(child_entity));

            Motor_t const world_transform =
                compute_world_transform(parent_transform, *child_transform);

            *child_transform = world_transform;

            compute_world_transforms_for_children(
                entity_manager,
                child_entity,
                world_transform,
                parent_to_children_map
            );
        }
    }

    /**
     * @brief Compute the world transforms for all entities in \p entity_manager.
     * 
     * @param entity_manager The container for all entitites and its components.
     * @param root_entities The entities that have no parent.
     * @param parent_to_children_map Map from entity to its children. 
     */
    export template <typename Translator_t, typename Rotor_t, typename Motor_t, typename Entity_manager_t>
    void compute_world_transforms(
        Entity_manager_t& entity_manager,
        std::span<Entity const> const root_entities,
        std::pmr::unordered_map<Entity, std::pmr::vector<Entity>, Entity_hash> const& parent_to_children_map
    ) noexcept
    {
        static_assert(!std::is_const_v<Entity_manager_t>);

        for_each(
            entity_manager.get_component_groups(),
            [] <typename T> (T& component_group) noexcept
            {
                if constexpr (contains<T, std::pmr::vector<Translator_t>, std::pmr::vector<Rotor_t>, std::pmr::vector<Motor_t>>())
                {
                    // Can be threaded
                    for (auto& pair : component_group)
                    {
                        std::span<Translator_t const> const translators =
                            std::get<std::pmr::vector<Translator_t>>(pair.second);

                        std::span<Rotor_t const> const rotors =
                            std::get<std::pmr::vector<Rotor_t>>(pair.second);

                        std::span<Motor_t> const motors =
                            std::get<std::pmr::vector<Motor_t>>(pair.second);

                        constexpr std::size_t number_of_threads = 12;
                        constexpr std::size_t minimum_amount_per_thread = 200;

                        auto do_task = [] (
                            std::span<Translator_t const> const translators,
                            std::span<Rotor_t const> const rotors,
                            std::span<Motor_t> const motors,
                            std::pair<std::size_t, std::size_t> const range
                        ) noexcept -> void
                        {
                            for (std::size_t index = range.first; index < range.second; ++index)
                            {
                                motors[index] = compute_transform(translators[index], rotors[index]);
                            }
                        };

                        std::pmr::vector<std::jthread> threads;
                        threads.reserve(number_of_threads);

                        for (std::size_t thread_index = 0; thread_index < number_of_threads; ++thread_index)
                        {
                            std::size_t const begin = thread_index * motors.size() / number_of_threads;
                            std::size_t const end = (thread_index + 1) * motors.size() / number_of_threads;

                            threads.emplace_back(
                                do_task,
                                translators,
                                rotors,
                                motors,
                                std::make_pair(begin, end)
                            );
                        }

                        /*// Can be threaded
                        for (std::size_t index = 0; index < motors.size(); ++index)
                        {
                            motors[index] = compute_transform(translators[index], rotors[index]);
                        }*/
                    }
                }
            }
        );

        // Can be threaded
        for (Entity const root_entity : root_entities)
        {
            Motor_t const root_transform =
                std::get<0>(entity_manager.get_components<Motor_t>(root_entity));

            compute_world_transforms_for_children(
                entity_manager,
                root_entity,
                root_transform,
                parent_to_children_map
            );
        }
    }
}
