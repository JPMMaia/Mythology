export module maia.ecs.world;

import maia.ecs.entity_manager;

namespace Maia::ECS
{
    export class World
    {
    public:

        Entity_manager& get_entity_manager() noexcept
        {
            return m_entity_manager;
        }

        Entity_manager const& get_entity_manager() const noexcept
        {
            return m_entity_manager;
        }

    private:

        Entity_manager m_entity_manager;

    };
}
