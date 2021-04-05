export module maia.ecs.entity_manager;

import maia.ecs.archetype;
import maia.ecs.component;
import maia.ecs.component_chunk_group;
import maia.ecs.components_view;
import maia.ecs.entity;
import maia.ecs.shared_component;

import <algorithm>;
import <any>;
import <cassert>;
import <cstddef>;
import <memory_resource>;
import <numeric>;
import <optional>;
import <span>;
import <unordered_map>;
import <vector>;

namespace Maia::ECS
{
    using Archetype_index = std::size_t;

    struct Entity_location_info
    {
        Archetype_index archetype_index;
        Chunk_group_hash chunk_group_hash;
        Component_chunk_group::Index chunk_group_index;
    };

    export using Shared_component_key = Chunk_group_hash;

    export template<Concept::Shared_component Shared_component_t>
    struct Default_shared_component_hash
    {
        std::size_t operator()(Shared_component_t const& shared_component) const noexcept
        {
            return 0; // TODO
        }
    };

    /**
     * @brief Container for entities, components and shared components.
     * 
     */
    export class Entity_manager
    {
    public:

        /**
         * @brief Construct an empty container.
         * 
         */
        Entity_manager() noexcept = default;
        
        /**
         * @brief Construct an empty container with custom allocators.
         * 
         * @param generic_allocator Allocator for all memory except component chunks memory.
         * @param component_chunks_allocator Allocator for component chunks memory.
         */
        Entity_manager(
            std::pmr::polymorphic_allocator<> const& generic_allocator,
            std::pmr::polymorphic_allocator<> const& component_chunks_allocator
        ) noexcept :
            m_generic_allocator{generic_allocator},
            m_component_chunks_allocator{component_chunks_allocator},
            m_archetypes{generic_allocator},
            m_component_chunk_groups{generic_allocator},
            m_entity_location_info{generic_allocator},
            m_free_entity_indices{generic_allocator},
            m_shared_components{generic_allocator}
        {
        }

        /**
         * @brief Construct a container and allocate as much memory as
         * possible.
         * 
         * @param archetypes Archetypes to add.
         * @param shared_component_keys The keys for the shared components.
         * @param number_of_entities_per_shared_component The number of 
         * entities associated with each shared component.
         * @param generic_allocator Allocator for all memory except component chunks memory.
         * @param component_chunks_allocator Allocator for component chunks memory.
         */
        Entity_manager(
            std::span<Archetype const> const archetypes,
            std::span<std::span<Shared_component_key const> const> const shared_component_keys,
            std::span<std::span<std::size_t const> const> const number_of_entities_per_shared_component,
            std::pmr::polymorphic_allocator<> const& generic_allocator,
            std::pmr::polymorphic_allocator<> const& component_chunks_allocator
        ) :
            Entity_manager(generic_allocator, component_chunks_allocator)
        {
            m_archetypes.reserve(archetypes.size());
            m_archetypes.insert(m_archetypes.end(), archetypes.begin(), archetypes.end());

            m_component_chunk_groups.reserve(archetypes.size());
            for (std::size_t archetype_index = 0; archetype_index < m_archetypes.size(); ++archetype_index)
            {
                Archetype const& archetype = archetypes[archetype_index];

                std::span<Component_type_size const> const component_type_sizes = archetype.get_component_type_sizes();
            
                std::size_t const total_component_size_in_bytes = 
                    std::accumulate(component_type_sizes.begin(), component_type_sizes.end(), sizeof(Entity));

                std::size_t const maximum_chunk_size_in_bytes = get_maximum_chunk_size_in_bytes();
                std::size_t const number_of_entities_per_chunk = maximum_chunk_size_in_bytes / total_component_size_in_bytes;
                
                std::span<Chunk_group_hash const> const group_hashes = shared_component_keys[archetype_index];
			    std::span<std::size_t const> const number_of_entities_per_group = number_of_entities_per_shared_component[archetype_index];

                Component_chunk_group chunk_group
                {
                    archetype.get_component_type_ids(),
                    component_type_sizes,
                    number_of_entities_per_chunk,
                    group_hashes,
                    number_of_entities_per_group,
                    component_chunks_allocator,
                    generic_allocator
                };

                m_component_chunk_groups.push_back(chunk_group);
            }

            {
                auto const number_of_entities_flat_view = 
                    number_of_entities_per_shared_component |
                    views::join;

                std::size_t const total_number_of_entities = std::accumulate(
                    std::begin(number_of_entities_flat_view),
                    std::end(number_of_entities_flat_view),
                    std::size_t{0}
                );

                m_entity_location_info.reserve(total_number_of_entities);
                m_free_entity_indices.reserve(total_number_of_entities);
            }

            {
                auto const shared_component_keys_flat_view = 
                    shared_component_keys |
                    views::join;

                // TODO remove unique?

                std::size_t const number_of_shared_components
                    = std::distance(std::begin(shared_component_keys_flat_view), std::end(shared_component_keys_flat_view));

                m_shared_components.reserve(number_of_shared_components);
            }
        }

        /**
         * @brief Get all archetypes.
         * 
         * @return The returned archetypes.
         */
        std::span<Archetype const> get_archetypes() const noexcept
        {
            return m_archetypes;
        }

        /**
         * @brief Get the archetype to which an entity belongs.
         * 
         * @param entity The entity.
         * @return The entity's archetype.
         */
        Archetype const& get_archetype(Entity const entity) const noexcept
        {
            static Archetype dummy;
            return dummy;
        }

        /**
         * @brief Create an entity and associated component data initialized to
         * 0.
         * 
         * @param archetype The archetype that describes the component data
         * types of the entity.
         * @return The created entity.
         */
        Entity create_entity(Archetype const& archetype)
        {
            Archetype_index const archetype_index =
                add_archetype_if_it_does_not_exist(archetype);

            constexpr Chunk_group_hash no_shared_component_hash{0};
            return create_entity(archetype_index, no_shared_component_hash);
        }

        /**
         * @brief Create an entity, associate it with a shared component and
         * initialize component data to 0.
         * 
         * @param archetype The archetype that describes the component data
         * types of the entity.
         * @param shared_component_key The key of the shared component.
         * @return The created entity.
         */
        Entity create_entity(
            Archetype const& archetype,
            Shared_component_key const shared_component_key
        )
        {
            Archetype_index const archetype_index =
                add_archetype_if_it_does_not_exist(archetype);

            return create_entity(archetype_index, shared_component_key);
        }

        /**
         * @brief Destroy an entity.
         * 
         * @param entity The entity to destroy.
         */
        void destroy_entity(Entity const entity)
        {
            Entity_location_info const& entity_location_info = m_entity_location_info[entity.value];

            Component_chunk_group& component_chunk_group =
                m_component_chunk_groups[entity_location_info.archetype_index];

            std::optional<Component_group_entity_moved> const entity_moved = component_chunk_group.remove_entity(
                entity_location_info.chunk_group_hash,
                entity_location_info.chunk_group_index
            );

            if (entity_moved)
            {
                Entity_location_info& entity_moved_location_info = m_entity_location_info[entity_moved->entity.value];
                entity_moved_location_info.chunk_group_index = entity_location_info.chunk_group_index;
            }

            m_entity_location_info[entity.value] = {};
            m_free_entity_indices.push_back(entity.value);
        }

        /**
         * @brief Get the number of entities described by an archetype.
         * 
         * @param archetype The archetype that describes the entity's component
         * data types.
         * @return The number of entities that have component data types that
         * match @archetype.
         */
        std::size_t get_number_of_entities(Archetype const& archetype) const noexcept
        {
            auto const location = std::find(m_archetypes.begin(), m_archetypes.end(), archetype);

            if (location != m_archetypes.end())
            {
                std::size_t const archetype_index = std::distance(m_archetypes.begin(), location);

                Component_chunk_group const& chunk_group = m_component_chunk_groups[archetype_index];
                return chunk_group.number_of_entities();
            }
            else
            {
                return 0;
        }
        }

        template<Concept::Component Component_t>
        void add_component_type(Entity const entity, std::pmr::polymorphic_allocator<std::byte> const& temporaries_allocator = {})
        {
            Entity_location_info const& entity_location_info = m_entity_location_info[entity.value];

            Archetype_index const original_archetype_index = entity_location_info.archetype_index;

            Archetype const& original_archetype = m_archetypes[original_archetype_index];

            std::span<Component_type_ID const> const original_component_type_ids = original_archetype.get_component_type_ids();
            std::span<Component_type_size const> const original_component_type_sizes = original_archetype.get_component_type_sizes();

            auto const to_component_type_info = [] (Component_type_ID const id, Component_type_size const size) -> Component_type_info
            {
                return {id, size};
            };

            std::pmr::vector<Component_type_info> new_component_type_infos{temporaries_allocator};
            new_component_type_infos.resize(original_component_type_ids.size() + 1);

            std::transform(
                original_component_type_ids.begin(),
                original_component_type_ids.end(),
                original_component_type_sizes.begin(),
                new_component_type_infos.begin(),
                to_component_type_info
            );

            new_component_type_infos.back() = create_component_type_info<Component_t>();

            sort_component_type_infos(new_component_type_infos.begin(), new_component_type_infos.end());

            Archetype new_archetype
            {
                new_component_type_infos,
                original_archetype.get_shared_component_type_id(),
                m_generic_allocator
            };

            Archetype_index const new_archetype_index = add_archetype_if_it_does_not_exist(std::move(new_archetype));

            move_entity(entity_location_info, m_component_chunk_groups[new_archetype_index]);

            Component_chunk_group& old_component_chunk_group = m_component_chunk_groups[original_archetype_index];
            // TODO remove archetype if no other entities are left
        }

        template<Concept::Component Component_t>
        void remove_component_type(Entity const entity)
        {
        }

        template<Concept::Shared_component Shared_component_t>
        void add_shared_component_type(Entity const entity, Shared_component_t const& shared_component)
        {
        }

        template<Concept::Shared_component Shared_component_t>
        void remove_shared_component_type(Entity const entity)
        {
        }

        /**
         * @brief Get the value of an entity's component. 
         * 
         * @tparam Component_t The component data type.
         * @param entity The entity.
         * @return The value of the entity's component data.
         */
        template<Concept::Component Component_t>
        Component_t get_component_value(Entity const entity) const noexcept
        {
            Entity_location_info const& entity_location_info = m_entity_location_info[entity.value];

            Component_chunk_group const& component_chunk_group =
                m_component_chunk_groups[entity_location_info.archetype_index];

            Component_t component = 
                component_chunk_group.get_component_value<Component_t>(
                    entity_location_info.chunk_group_hash,
                    entity_location_info.chunk_group_index
                );

            return component;
        }

        /**
         * @brief Set the value of an entity's component.
         * 
         * @tparam Component_t The component data type.
         * @param entity The entity.
         * @param value The value of the entity's component data.
         */
        template<Concept::Component Component_t>
        void set_component_value(Entity const entity, Component_t const& value) noexcept
        {
            Entity_location_info const& entity_location_info = m_entity_location_info[entity.value];

            Component_chunk_group& component_chunk_group =
                m_component_chunk_groups[entity_location_info.archetype_index];

            component_chunk_group.set_component_value<Component_t>(
                entity_location_info.chunk_group_hash,
                entity_location_info.chunk_group_index,
                value
            );
        }

        /**
         * @brief Get the shared component data associated with an entity.
         * 
         * @tparam Shared_component_t The shared component data type.
         * @param entity The entity.
         * @return The value of the entity's shared component.
         */
        template<Concept::Shared_component Shared_component_t>
        Shared_component_t const& get_shared_component(
            Entity const entity
        ) const noexcept
        {
            Chunk_group_hash const key = m_entity_location_info[entity.value].chunk_group_hash;

            return get_shared_component<Shared_component_t>(key);
        }

        /**
         * @brief Get the shared component data associated with a key.
         * 
         * @tparam Shared_component_t The shared component data type.
         * @param shared_component_key The shared component's key.
         * @return The value of the shared component.
         */
        template<Concept::Shared_component Shared_component_t>
        Shared_component_t const& get_shared_component(
            Shared_component_key const shared_component_key
        ) const noexcept
        {
            std::any const& value = m_shared_components.at(shared_component_key);

            return *std::any_cast<Shared_component_t>(&value);
        }

        /**
         * @brief Set the shared component value for a given key.
         * 
         * @tparam Shared_component_t The shared component data type.
         * @param shared_component_key The shared component's key.
         * @param shared_component The value of the shared component.
         */
        template<Concept::Shared_component Shared_component_t>
        void set_shared_component(
            Shared_component_key const shared_component_key,
            Shared_component_t shared_component
        ) noexcept
        {
            m_shared_components[shared_component_key] = shared_component;
        }

        /**
         * @brief Change the entity's shared component data.
         * 
         * @param entity The entity to change.
         * @param shared_component_key The new shared component key.
         */
        void change_entity_shared_component(Entity const entity, Shared_component_key const shared_component_key) noexcept
        {
            Entity_location_info& location_info = m_entity_location_info[entity.value];
            
            Chunk_group_hash const old_key = location_info.chunk_group_hash;
            Chunk_group_hash const new_key = shared_component_key;

            Component_chunk_group& chunk_group = m_component_chunk_groups[location_info.archetype_index];
            Entity_move_result const result = chunk_group.move_entity(old_key, location_info.chunk_group_index, new_key);

            if (result.entity_moved_by_remove.has_value())
            {
                Entity const moved_entity = result.entity_moved_by_remove->entity;
                
                Entity_location_info& moved_location_info = m_entity_location_info[moved_entity.value];

                Component_chunk_group::Index const new_moved_entity_index = location_info.chunk_group_index;
                moved_location_info.chunk_group_index = new_moved_entity_index;
            }

            location_info.chunk_group_hash = shared_component_key;
            location_info.chunk_group_index = result.new_index;
        }

        /*std::span<Component_chunk_view> get_component_chunk_views(Archetype const archetype) noexcept
        {
            return {};
        }

        template<typename... Archetype_infos>
        static constexpr std::size_t get_generic_required_memory_size(Archetype_infos const&... archetype_infos) noexcept
        {
            return 20000;
        }

    private:

        Archetype_index add_archetype(Archetype archetype)
        {
            m_component_chunk_groups.push_back(
                create_component_chunk_group(archetype, m_generic_allocator, m_component_chunks_allocator)
            );

            m_archetypes.push_back(std::move(archetype));

            return m_archetypes.size() - 1;
        }

        static Component_chunk_group create_component_chunk_group(
            Archetype const& archetype,
            std::pmr::polymorphic_allocator<std::byte> const& generic_allocator,
            std::pmr::polymorphic_allocator<std::byte> const& component_chunks_allocator
        )
        {
            std::span<Component_type_size const> const component_type_sizes = archetype.get_component_type_sizes();
            
            std::size_t const total_component_size_in_bytes = 
                std::accumulate(component_type_sizes.begin(), component_type_sizes.end(), sizeof(Entity));

            constexpr std::size_t maximum_chunk_size_in_bytes = 16 * 1024;
            std::size_t const number_of_entities_per_chunk = maximum_chunk_size_in_bytes / total_component_size_in_bytes;

            return Component_chunk_group
                {
                    archetype.get_component_type_ids(),
                    component_type_sizes,
                    number_of_entities_per_chunk,
                component_chunks_allocator,
                generic_allocator
            };
        }

        Archetype_index add_archetype_if_it_does_not_exist(Archetype const& archetype)
        {
            std::optional<Archetype_index> const archetype_index =
                find_archetype_index(archetype);

            if (archetype_index.has_value()) [[likely]]
            {
                return *archetype_index;
            }
            else
            {
                return add_archetype(archetype);
            }
        }

        std::optional<Archetype_index> find_archetype_index(Archetype const& archetype) const noexcept
        {
            auto const archetype_iterator = std::find(m_archetypes.begin(), m_archetypes.end(), archetype);

            if (archetype_iterator != m_archetypes.end())
            {
                return std::distance(m_archetypes.begin(), archetype_iterator);
            }
            else
            {
                return {};
            }
        }

        Entity::Integral_type get_next_entity_value() const noexcept
        {
            if (!m_free_entity_indices.empty())
            {
                return m_free_entity_indices.back();
        }
            else
            {
                return m_entity_location_info.size();
            }
        }

        void increment_next_entity_value() noexcept
        {
            if (!m_free_entity_indices.empty())
            {
                m_free_entity_indices.pop_back();
            }
        }

        Entity create_entity(Archetype_index const archetype_index, Chunk_group_hash const chunk_group_hash)
        {
            Entity::Integral_type const new_entity_value = get_next_entity_value();
            increment_next_entity_value();

            Entity const new_entity{new_entity_value};
  
            Component_chunk_group& component_chunk_group = m_component_chunk_groups[archetype_index];

            Component_chunk_group::Index const component_chunk_group_index = 
                component_chunk_group.add_entity(new_entity, chunk_group_hash);

            Entity_location_info const entity_location_info
                {
                    archetype_index,
                    chunk_group_hash,
                    component_chunk_group_index
            };

            if (new_entity_value < m_entity_location_info.size())
            {
                m_entity_location_info[new_entity_value] = entity_location_info;
                }
            else
            {
                assert(new_entity_value == m_entity_location_info.size());

                m_entity_location_info.push_back(entity_location_info);
            }
            
            return new_entity;
        }

        void move_entity(Entity_location_info const& entity_location_info, Component_chunk_group& to)
        {
            Component_chunk_group& from = m_component_chunk_groups[entity_location_info.archetype_index];

            // TODO

            // Create new entity in to component chunk group

            // For each old component, read from old chunk and write to new chunk

            // Remove entity from from component chunk group
        }

        std::pmr::polymorphic_allocator<std::byte> m_generic_allocator;
        std::pmr::polymorphic_allocator<std::byte> m_component_chunks_allocator;
        
        std::pmr::vector<Archetype> m_archetypes;
        std::pmr::vector<Component_chunk_group> m_component_chunk_groups;

        std::pmr::vector<Entity_location_info> m_entity_location_info;
        std::pmr::vector<std::size_t> m_free_entity_indices;

        std::pmr::unordered_map<Chunk_group_hash, std::any> m_shared_components;
    };
}
