export module maia.ecs.component_chunk;

import maia.ecs.component;
import maia.ecs.entity;

import <memory_resource>;

namespace Maia::ECS
{
    export class Component_chunk
    {
    public:

        explicit Component_chunk(std::pmr::polymorphic_allocator<std::byte> const& chunk_allocator)
        {
        }

        void add_entity(Entity const entity) noexcept
        {
        }

        void remove_entity(Entity const entity) noexcept
        {
        }

        template<Concept::Component Component_t>
        Component_t get_component_value(Entity const entity) const noexcept
        {
            return {};
        }

        template<Concept::Component Component_t>
        void set_component_value(Entity const entity, Component_t const value) const noexcept
        {
            return {};
        }
    };
}
