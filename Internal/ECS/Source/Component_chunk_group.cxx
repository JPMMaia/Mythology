export module maia.ecs.component_chunk_group;

import maia.ecs.component;
import maia.ecs.component_chunk;
import maia.ecs.entity;

import <array>;
import <cstddef>;
import <memory_resource>;
import <span>;
import <vector>;

namespace Maia::ECS
{
    export class Component_chunk_group
    {
    public:

        Component_chunk_group(
            std::span<Component_type_ID const> const component_type_ids,
            std::pmr::polymorphic_allocator<std::byte> const& generic_allocator,
            std::pmr::polymorphic_allocator<std::byte> const& component_chunks_allocator
        ) :
            m_component_type_ids{component_type_ids.begin(), component_type_ids.end(), generic_allocator}
        {
        }

        void add_entity(Entity const entity)
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

        template<Concept::Component Component_t>
        bool has_component() const noexcept
        {
            Component_type_ID const component_type_id = get_component_type_id<Component_t>();
            return has_component(component_type_id);
        }

        bool has_component(Component_type_ID const component_type_id) const noexcept
        {
            auto const iterator = std::find(m_component_type_ids.begin(), m_component_type_ids.end(), component_type_id);

            return iterator != m_component_type_ids.end();
        }

    private:

        std::pmr::vector<Component_type_ID> m_component_type_ids;
    };

    export template<Concept::Component... Component_t>
        Component_chunk_group create_component_chunk_group(
            std::pmr::polymorphic_allocator<std::byte> const& generic_allocator,
            std::pmr::polymorphic_allocator<std::byte> const& component_chunks_allocator
        )
    {
        std::array<Component_type_ID, sizeof...(Component_t)> const component_type_ids
        {
            (get_component_type_id<Component_t>(),...)
        };

        return Component_chunk_group
        {
            component_type_ids,
            generic_allocator,
            component_chunks_allocator
        };
    }
}
