export module maia.ecs.world;

import maia.ecs.archetype;
import maia.ecs.component;
import maia.ecs.components_view;
import maia.ecs.entity;
import maia.ecs.shared_component;

import <span>;

namespace Maia::ECS
{
    export class World
    {
    public:

        Archetype create_archetype(std::span<Component_type_ID const> component_ids)
        {
            return {};
        }

        Archetype create_archetype(Shared_component_type_ID const shared_component_id, std::span<Component_type_ID const> component_ids)
        {
            return {};
        }

        std::span<Archetype const> get_archetypes() const noexcept
        {
            return {};
        }

        Entity create_entity(Archetype const& archetype)
        {
            return {};
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
        Shared_component_t& create_shared_component(Shared_component_t&& shared_component)
        {
            static Shared_component_t dummy;
            return dummy;
        }

        std::span<Component_chunk_view const> get_component_chunk_views(Archetype const archetype) const noexcept
        {
            return {};
        }
    };
}
