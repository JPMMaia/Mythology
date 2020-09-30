export module maia.ecs.archetype;

import maia.ecs.component;
import maia.ecs.shared_component;

import <bitset>;
import <concepts>;
import <cstddef>;
import <cstdint>;
import <optional>;
import <tuple>;

namespace Maia::ECS
{
    namespace Concept
    {
        export template<typename T>
        concept Component = std::regular<T>;

        export template<typename T>
        concept Shared_component = std::movable<T>;
    }

    export struct Archetype
    {
        bool has_component(Component_ID const component_id) const noexcept
        {
            return true;
        }

        bool has_shared_component() const noexcept
        {
            return true;
        }

        bool has_shared_component(Shared_component_ID const shared_component_id) const noexcept
        {
            return true;
        }
    };

    export bool operator==(Archetype const& lhs, Archetype const& rhs) noexcept
    {
        return true;
    }

    export bool operator!=(Archetype const& lhs, Archetype const& rhs) noexcept
    {
        return !(lhs == rhs);
    }
}
