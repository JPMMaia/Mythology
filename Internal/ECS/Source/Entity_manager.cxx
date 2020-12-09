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
import <optional>;
import <span>;
import <vector>;

namespace Maia::ECS
{
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
            assert(!archetype.has_shared_component());

            std::optional<Archetype_index> const archetype_index = find_archetype_index(archetype);

            if (archetype_index)
            {
                Component_chunk_group& chunk_group = m_component_chunk_groups.at(*archetype_index);

                return create_entity(chunk_group);
            }
            else
            {
                m_archetypes.push_back(archetype);
                
                std::span<Component_type_ID const> const component_type_ids = 
                    archetype.get_component_type_ids();

                m_component_chunk_groups.push_back(
                    Component_chunk_group{component_type_ids, {}, {}}
                );

                Component_chunk_group& chunk_group = m_component_chunk_groups.back();

                return create_entity(chunk_group);
            }
        }

        template<Concept::Shared_component Shared_component_t>
        Entity create_entity(Archetype const& archetype, Shared_component_t const& shared_component)
        {
            assert(archetype.has_shared_component());

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
            return {};
        }

        template<Concept::Component Component_t>
        void set_component_value(Entity const entity, Component_t const value) noexcept
        {
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

        using Archetype_index = std::iterator_traits<std::vector<Archetype>::const_iterator>::difference_type;

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

        Entity create_entity(Component_chunk_group& component_chunk_group)
        {
            Entity::Integral_type const new_entity_value = get_next_entity_value();
            increment_next_entity_value();

            Entity const new_entity{new_entity_value};
            
            component_chunk_group.add_entity(new_entity);
            
            return new_entity;
        }

        std::pmr::polymorphic_allocator<std::byte> m_generic_allocator;
        std::pmr::polymorphic_allocator<std::byte> m_component_chunks_allocator;
        std::pmr::vector<Archetype> m_archetypes;
        std::pmr::vector<Component_chunk_group> m_component_chunk_groups;
        Entity::Integral_type m_next_entity_value;
    };
}
