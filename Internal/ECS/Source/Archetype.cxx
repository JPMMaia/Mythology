export module maia.ecs.archetype;

import maia.ecs.component;
import maia.ecs.shared_component;

import <algorithm>;
import <cassert>;
import <cstddef>;
import <cstdint>;
import <memory_resource>;
import <optional>;
import <ranges>;
import <span>;
import <vector>;

namespace Maia::ECS
{
    namespace
    {
        std::pmr::vector<Component_type_ID> create_component_type_ids(
            std::span<Component_type_info const> const component_type_infos,
            std::pmr::polymorphic_allocator<Component_type_ID> const& allocator
        )
        {
            auto const to_component_type_id = [] (Component_type_info const type_info) -> Component_type_ID
            {
                return type_info.id;
            };

            auto const ids_view = component_type_infos | std::views::transform(to_component_type_id);

            return {ids_view.begin(), ids_view.end(), allocator};
        }

        std::pmr::vector<Component_type_size> create_component_type_sizes(
            std::span<Component_type_info const> const component_type_infos,
            std::pmr::polymorphic_allocator<Component_type_ID> const& allocator
        )
        {
            auto const to_component_type_size = [] (Component_type_info const type_info) -> Component_type_size
            {
                return type_info.size;
            };

            auto const sizes_view = component_type_infos | std::views::transform(to_component_type_size);

            return {sizes_view.begin(), sizes_view.end(), allocator};
        }
    }

    export class Archetype
    {
    public:

        Archetype() noexcept = default;

        Archetype(
            std::span<Component_type_info const> const component_type_infos,
            std::optional<Shared_component_type_ID> const shared_component_type_id,
            std::pmr::polymorphic_allocator<std::byte> const& allocator
        ) :
            m_component_type_ids{create_component_type_ids(component_type_infos, allocator)},
            m_component_type_sizes{create_component_type_sizes(component_type_infos, allocator)},
            m_shared_component_type_id{shared_component_type_id}
        {
        }

        Archetype(
            std::span<Component_type_ID const> const component_type_ids,
            std::span<Component_type_size const> const component_type_sizes,
            std::optional<Shared_component_type_ID> const shared_component_type_id,
            std::pmr::polymorphic_allocator<std::byte> const& allocator
        ) :
            m_component_type_ids{component_type_ids.begin(), component_type_ids.end(), allocator},
            m_component_type_sizes{component_type_sizes.begin(), component_type_sizes.end(), allocator},
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

        std::span<Component_type_size const> get_component_type_sizes() const noexcept
        {
            return m_component_type_sizes;
        }

        std::optional<Shared_component_type_ID> get_shared_component_type_id() const noexcept
        {
            return m_shared_component_type_id;
        }

    private:

        std::pmr::vector<Component_type_ID> m_component_type_ids;
        std::pmr::vector<Component_type_size> m_component_type_sizes;
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
