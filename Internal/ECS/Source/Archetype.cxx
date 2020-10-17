export module maia.ecs.archetype;

import maia.ecs.component;
import maia.ecs.shared_component;

import <bitset>;
import <cassert>;
import <cstddef>;
import <cstdint>;
import <optional>;
import <span>;
import <tuple>;

namespace Maia::ECS
{
    export struct Archetype
    {
        bool has_component(Component_type_ID const component_type_id) const noexcept
        {
            assert(component_type_id.value < this->component_type_ids.size());

            return this->component_type_ids.test(component_type_id.value);
        }

        bool has_shared_component() const noexcept
        {
            return this->shared_component_type_id.has_value();
        }

        bool has_shared_component(Shared_component_type_ID const shared_component_type_id) const noexcept
        {
            return this->shared_component_type_id.has_value() ?
                *this->shared_component_type_id == shared_component_type_id :
                false;
        }

        std::bitset<496> component_type_ids;
        std::optional<Shared_component_type_ID> shared_component_type_id;
    };

    export Archetype create_archetype(std::span<Component_type_ID const> const component_type_ids) noexcept
    {
        Archetype archetype{};
        
        for (Component_type_ID const component_type_id : component_type_ids)
        {
            assert(component_type_id.value < archetype.component_type_ids.size());

            archetype.component_type_ids.set(component_type_id.value);
        }

        return archetype;
    }

    export Archetype create_archetype(Shared_component_type_ID const shared_component_type_id, std::span<Component_type_ID const> const component_type_ids) noexcept
    {
        Archetype archetype = create_archetype(component_type_ids);
        archetype.shared_component_type_id = shared_component_type_id;

        return archetype;
    }

    export bool operator==(Archetype const& lhs, Archetype const& rhs) noexcept
    {
        return lhs.component_type_ids == rhs.component_type_ids;
    }

    export bool operator!=(Archetype const& lhs, Archetype const& rhs) noexcept
    {
        return !(lhs == rhs);
    }
}
