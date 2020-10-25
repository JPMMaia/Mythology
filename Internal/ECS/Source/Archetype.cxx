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
    namespace
    {
        std::vector<Component_type_ID> create_sorted_component_type_ids(
            std::span<Component_type_ID const> const component_type_ids,
            std::pmr::polymorphic_allocator<Component_type_ID> const& allocator
        )
        {
            std::vector<Component_type_ID> vector;
            vector.resize(component_type_ids.size());
            std::copy(component_type_ids.begin(), component_type_ids.end(), vector.begin());

            std::sort(vector.begin(), vector.end());

            return vector;
        }
    }

    export class Archetype
    {
    public:

        Archetype() noexcept = default;

        Archetype(
            std::span<Component_type_ID const> const component_type_ids,
            std::pmr::polymorphic_allocator<std::byte> const& allocator
        ) :
            m_component_type_ids{create_sorted_component_type_ids(component_type_ids, allocator)},
            m_shared_component_type_id{std::nullopt}
        {
        }

        Archetype(
            Shared_component_type_ID const shared_component_type_id,
            std::span<Component_type_ID const> const component_type_ids,
            std::pmr::polymorphic_allocator<std::byte> const& allocator
        ) :
            m_component_type_ids{create_sorted_component_type_ids(component_type_ids, allocator)},
            m_shared_component_type_id{shared_component_type_id}
        {
        }


        bool has_component(Component_type_ID const component_type_id) const noexcept
        {
            auto const location = std::find(m_component_type_ids.begin(), m_component_type_ids.end(), component_type_id);

            return location != m_component_type_ids.end();
        }

        bool has_shared_component() const noexcept
        {
            return m_shared_component_type_id.has_value();
        }

        bool has_shared_component(Shared_component_type_ID const shared_component_type_id) const noexcept
        {
            return m_shared_component_type_id.has_value() ?
                *m_shared_component_type_id == shared_component_type_id :
                false;
        }

        std::span<Component_type_ID const> get_component_type_ids() const noexcept
        {
            return m_component_type_ids;
        }

        std::optional<Shared_component_type_ID> get_shared_component_type_id() const noexcept
        {
            return m_shared_component_type_id;
        }

        std::vector<Component_type_ID> m_component_type_ids; // TODO change to pmr
        std::optional<Shared_component_type_ID> m_shared_component_type_id;
    };

    export bool operator==(Archetype const& lhs, Archetype const& rhs) noexcept
    {
        if (lhs.get_shared_component_type_id() != rhs.get_shared_component_type_id())
        {
            return false;
        }

        std::span<Component_type_ID const> const lhs_component_type_ids = lhs.get_component_type_ids();
        std::span<Component_type_ID const> const rhs_component_type_ids = rhs.get_component_type_ids();

        if (lhs_component_type_ids.size() != rhs_component_type_ids.size())
        {
            return false;
        }

        return std::equal(
            lhs_component_type_ids.begin(),
            lhs_component_type_ids.end(),
            rhs_component_type_ids.begin()
        );
    }

    export bool operator!=(Archetype const& lhs, Archetype const& rhs) noexcept
    {
        return !(lhs == rhs);
    }
}
