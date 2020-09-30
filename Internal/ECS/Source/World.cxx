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

        Archetype create_archetype(std::span<Component_ID const> component_ids)
        {
            return {};
        }

        Archetype create_archetype(Shared_component_ID const shared_component_id, std::span<Component_ID const> component_ids)
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

        Components_view get_components_views(std::span<Archetype const> const archetypes) const noexcept
        {
            return {};
        }
    };
}
