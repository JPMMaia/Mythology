export module maia.ecs.entity_manager;

import maia.ecs.archetype;
import maia.ecs.component;
import maia.ecs.component_chunk_group;
import maia.ecs.components_view;
import maia.ecs.entity;
import maia.ecs.shared_component;

import <algorithm>;
import <cassert>;
import <cstddef>;
import <memory_resource>;
import <numeric>;
import <optional>;
import <span>;
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

    export class Entity_manager
    {
    public:

        std::span<Archetype const> get_archetypes() const noexcept
        {
            return {};
        }

        Archetype const& get_archetype(Entity const entity) const noexcept
        {
            static Archetype dummy;
            return dummy;
        }

        Entity create_entity(Archetype const& archetype)
        {
            Archetype_index const archetype_index =
                add_archetype_if_it_does_not_exist(archetype);

            constexpr Chunk_group_hash no_shared_component_hash{0};
            return create_entity(archetype_index, no_shared_component_hash);
        }

        template<Concept::Shared_component Shared_component_t>
        Entity create_entity(Archetype const& archetype, Shared_component_t const& shared_component)
        {
            return {};
        }

        void destroy_entity(Entity const entity)
        {
        }

        template<Concept::Component Component_t>
        void add_component_type(Entity const entity)
        {
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

        template<Concept::Shared_component Shared_component_t>
        Shared_component_t const& get_shared_component_value(Entity const entity) const noexcept
        {
            static Shared_component_t dummy;
            return dummy;
        }

        template<Concept::Shared_component Shared_component_t>
        void set_shared_component_value(Entity const entity, Shared_component_t const& value) noexcept
        {
        }

        /*std::span<Component_chunk_view> get_component_chunk_views(Archetype const archetype) noexcept
        {
            return {};
        }

        std::span<Component_chunk_view const> get_component_chunk_views(Archetype const archetype) const noexcept
        {
            return {};
        }*/

    private:

        Archetype_index add_archetype(Archetype const& archetype)
        {
            m_archetypes.push_back(archetype);

            std::span<Component_type_size const> const component_type_sizes = archetype.get_component_type_sizes();
            
            std::size_t const total_component_size_in_bytes = 
                std::accumulate(component_type_sizes.begin(), component_type_sizes.end(), sizeof(Entity));

            constexpr std::size_t maximum_chunk_size_in_bytes = 16 * 1024;
            std::size_t const number_of_entities_per_chunk = maximum_chunk_size_in_bytes / total_component_size_in_bytes;

            m_component_chunk_groups.push_back(
                Component_chunk_group
                {
                    archetype.get_component_type_ids(),
                    component_type_sizes,
                    number_of_entities_per_chunk,
                    m_component_chunks_allocator,
                    m_generic_allocator
                }
            );

            return m_archetypes.size() - 1;
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
            return m_next_entity_value;
        }

        void increment_next_entity_value() noexcept
        {
            ++m_next_entity_value;
        }

        Entity create_entity(Archetype_index const archetype_index, Chunk_group_hash const chunk_group_hash)
        {
            Entity::Integral_type const new_entity_value = get_next_entity_value();
            increment_next_entity_value();

            Entity const new_entity{new_entity_value};
  
            Component_chunk_group& component_chunk_group = m_component_chunk_groups[archetype_index];

            Component_chunk_group::Index component_chunk_group_index = 
                component_chunk_group.add_entity(new_entity, chunk_group_hash);

            m_entity_location_info.push_back(
                Entity_location_info
                {
                    archetype_index,
                    chunk_group_hash,
                    component_chunk_group_index
                }
            );
            
            return new_entity;
        }

        std::pmr::polymorphic_allocator<std::byte> m_generic_allocator;
        std::pmr::polymorphic_allocator<std::byte> m_component_chunks_allocator;
        
        std::pmr::vector<Archetype> m_archetypes;
        std::pmr::vector<Component_chunk_group> m_component_chunk_groups;

        std::pmr::vector<Entity_location_info> m_entity_location_info;
        
        Entity::Integral_type m_next_entity_value;
    };
}
