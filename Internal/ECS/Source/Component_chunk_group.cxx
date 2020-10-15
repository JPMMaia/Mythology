export module maia.ecs.component_chunk_group;

import maia.ecs.component;
import maia.ecs.entity;

namespace Maia::ECS
{
    export class Component_chunk_group
    {
    public:

        void add_entity(Entity const entity)
        {
        }

        template<Concept::Component Component_t>
        Component_t get_component_value() noexcept
        {
            return {};
        }
    
        template<Concept::Component Component_t>
        bool has_component() noexcept
        {
            return false;
        }
    };

    export template<Concept::Component... Component_t>
    Component_chunk_group create_component_chunk_group()
    {
        return {};
    }
}
