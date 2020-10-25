export module maia.ecs.archetype;

import maia.ecs.component;
import maia.ecs.shared_component;

import <algorithm>;
import <cassert>;
import <cstddef>;
import <cstdint>;
import <memory_resource>;
import <optional>;
import <span>;
import <vector>;

namespace Maia::ECS
{
    export struct Archetype
    {
        bool has_component(Component_type_ID const component_type_id) const noexcept
        {
            auto const location = std::find(this->component_type_ids.begin(), this->component_type_ids.end(), component_type_id);

            return location != this->component_type_ids.end();
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

        std::span<Component_type_ID const> get_component_type_ids() const noexcept
        {
            return this->component_type_ids;
        }

        std::vector<Component_type_ID> component_type_ids; // TODO change to pmr
        std::optional<Shared_component_type_ID> shared_component_type_id;
    };

    export bool operator==(Archetype const& lhs, Archetype const& rhs) noexcept
    {
        return lhs.component_type_ids == rhs.component_type_ids;
    }

    export bool operator!=(Archetype const& lhs, Archetype const& rhs) noexcept
    {
        return !(lhs == rhs);
    }


    export Archetype create_archetype(
        std::span<Component_type_ID const> const component_type_ids,
        std::pmr::polymorphic_allocator<Component_type_ID> const& allocator
    ) noexcept
    {
        std::vector<Component_type_ID> vector;
        vector.resize(component_type_ids.size());
        std::copy(component_type_ids.begin(), component_type_ids.end(), vector.begin());

        return
        {
            .component_type_ids = std::move(vector),
            .shared_component_type_id = std::nullopt
        };
    }

    export Archetype create_archetype(
        Shared_component_type_ID const shared_component_type_id,
        std::span<Component_type_ID const> const component_type_ids,
        std::pmr::polymorphic_allocator<std::byte> const& allocator) noexcept
    {
        Archetype archetype = create_archetype(component_type_ids, allocator);
        archetype.shared_component_type_id = shared_component_type_id;

        return archetype;
    }
}
