export module maia.ecs.archetype;

import maia.ecs.component;
import maia.ecs.shared_component;

import <bitset>;
import <cstddef>;
import <cstdint>;
import <optional>;
import <span>;
import <tuple>;

namespace Maia::ECS
{
    export struct Archetype
    {
        bool has_component(Component_type_ID const component_id) const noexcept
        {
            return true;
        }

        bool has_shared_component() const noexcept
        {
            return true;
        }

        bool has_shared_component(Shared_component_type_ID const shared_component_id) const noexcept
        {
            return true;
        }
    };

    export Archetype create_archetype(std::span<Component_type_ID const> component_ids) noexcept
    {
        return {};
    }

    export Archetype create_archetype(Shared_component_type_ID const shared_component_id, std::span<Component_type_ID const> component_ids) noexcept
    {
        return {};
    }

    export bool operator==(Archetype const& lhs, Archetype const& rhs) noexcept
    {
        return true;
    }

    export bool operator!=(Archetype const& lhs, Archetype const& rhs) noexcept
    {
        return !(lhs == rhs);
    }
}
