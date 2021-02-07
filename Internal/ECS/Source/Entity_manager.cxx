export module maia.ecs.entity_manager;

import maia.ecs.archetype;
import maia.ecs.component;
import maia.ecs.component_chunk_group;
import maia.ecs.components_view;
import maia.ecs.entity;
import maia.ecs.shared_component;

import <algorithm>;
import <any>;
import <cassert>;
import <cstddef>;
import <memory_resource>;
import <numeric>;
import <optional>;
import <span>;
import <unordered_map>;
import <vector>;

namespace Maia::ECS
{
    using Archetype_index = std::size_t;

    struct Entity_location_info
    {
        Archetype_index archetype_index;
        Chunk_group_hash chunk_group_hash;
        Component_chunk_group::Index chunk_group_index;
    };

    export using Shared_component_key = Chunk_group_hash;

    export template<Concept::Shared_component Shared_component_t>
    struct Default_shared_component_hash
    {
        std::size_t operator()(Shared_component_t const& shared_component) const noexcept
        {
            return 0; // TODO
        }
    };

    export class Entity_manager
    {
    public:

        std::span<Archetype const> get_archetypes() const noexcept
        {
            return m_archetypes;
        }

        Archetype const& get_archetype(Entity const entity) const noexcept
        {
            static Archetype dummy;
            return dummy;
        }

        Entity create_entity(Archetype const& archetype)
        {
            Archetype_index const archetype_index =
                add_archetype_if_it_does_not_exist(archetype);

            constexpr Chunk_group_hash no_shared_component_hash{0};
            return create_entity(archetype_index, no_shared_component_hash);
        }

        Entity create_entity(
            Archetype const& archetype,
            Shared_component_key const shared_component_key
        )
        {
            Archetype_index const archetype_index =
                add_archetype_if_it_does_not_exist(archetype);

            return create_entity(archetype_index, shared_component_key);
        }

        void destroy_entity(Entity const entity)
        {
            Entity_location_info const& entity_location_info = m_entity_location_info[entity.value];

            Component_chunk_group& component_chunk_group =
                m_component_chunk_groups[entity_location_info.archetype_index];

            std::optional<Component_group_entity_moved> const entity_moved = component_chunk_group.remove_entity(
                entity_location_info.chunk_group_hash,
                entity_location_info.chunk_group_index
            );

            if (entity_moved)
            {
                Entity_location_info& entity_moved_location_info = m_entity_location_info[entity_moved->entity.value];
                entity_moved_location_info.chunk_group_index = entity_location_info.chunk_group_index;
            }

            // TODO invalidate entity and free its info from m_entity_location_info
        }

        std::size_t get_number_of_entities(Archetype const& archetype) const noexcept
        {
            auto const location = std::find(m_archetypes.begin(), m_archetypes.end(), archetype);

            if (location != m_archetypes.end())
            {
                std::size_t const archetype_index = std::distance(m_archetypes.begin(), location);

                Component_chunk_group const& chunk_group = m_component_chunk_groups[archetype_index];
                return chunk_group.number_of_entities();
            }
            else
            {
                return 0;
        }
        }

        template<Concept::Component Component_t>
        void add_component_type(Entity const entity, std::pmr::polymorphic_allocator<std::byte> const& temporaries_allocator = {})
        {
            Entity_location_info const& entity_location_info = m_entity_location_info[entity.value];

            Archetype_index const original_archetype_index = entity_location_info.archetype_index;

            Archetype const& original_archetype = m_archetypes[original_archetype_index];

            std::span<Component_type_ID const> const original_component_type_ids = original_archetype.get_component_type_ids();
            std::span<Component_type_size const> const original_component_type_sizes = original_archetype.get_component_type_sizes();

            auto const to_component_type_info = [] (Component_type_ID const id, Component_type_size const size) -> Component_type_info
            {
                return {id, size};
            };

            std::pmr::vector<Component_type_info> new_component_type_infos{temporaries_allocator};
            new_component_type_infos.resize(original_component_type_ids.size() + 1);

            std::transform(
                original_component_type_ids.begin(),
                original_component_type_ids.end(),
                original_component_type_sizes.begin(),
                new_component_type_infos.begin(),
                to_component_type_info
            );

            new_component_type_infos.back() = create_component_type_info<Component_t>();

            sort_component_type_infos(new_component_type_infos.begin(), new_component_type_infos.end());

            Archetype new_archetype
            {
                new_component_type_infos,
                original_archetype.get_shared_component_type_id(),
                m_generic_allocator
            };

            Archetype_index const new_archetype_index = add_archetype_if_it_does_not_exist(std::move(new_archetype));

            move_entity(entity_location_info, m_component_chunk_groups[new_archetype_index]);

            Component_chunk_group& old_component_chunk_group = m_component_chunk_groups[original_archetype_index];
            // TODO remove archetype if no other entities are left
        }

        template<Concept::Component Component_t>
        void remove_component_type(Entity const entity)
        {
        }

        template<Concept::Shared_component Shared_component_t>
        void add_shared_component_type(Entity const entity, Shared_component_t const& shared_component)
        {
        }

        template<Concept::Shared_component Shared_component_t>
        void remove_shared_component_type(Entity const entity)
        {
        }

        template<Concept::Component Component_t>
        Component_t get_component_value(Entity const entity) const noexcept
        {
            Entity_location_info const& entity_location_info = m_entity_location_info[entity.value];

            Component_chunk_group const& component_chunk_group =
                m_component_chunk_groups[entity_location_info.archetype_index];

            Component_t component = 
                component_chunk_group.get_component_value<Component_t>(
                    entity_location_info.chunk_group_hash,
                    entity_location_info.chunk_group_index
                );

            return component;
        }

        template<Concept::Component Component_t>
        void set_component_value(Entity const entity, Component_t const& value) noexcept
        {
            Entity_location_info const& entity_location_info = m_entity_location_info[entity.value];

            Component_chunk_group& component_chunk_group =
                m_component_chunk_groups[entity_location_info.archetype_index];

            component_chunk_group.set_component_value<Component_t>(
                entity_location_info.chunk_group_hash,
                entity_location_info.chunk_group_index,
                value
            );
        }

        template<Concept::Shared_component Shared_component_t>
        Shared_component_t const& get_shared_component(
            Entity const entity
        ) const noexcept
        {
            Chunk_group_hash const key = m_entity_location_info[entity.value].chunk_group_hash;

            return get_shared_component<Shared_component_t>(key);
        }

        template<Concept::Shared_component Shared_component_t>
        Shared_component_t const& get_shared_component(
            Shared_component_key const shared_component_key
        ) const noexcept
        {
            std::any const& value = m_shared_components.at(shared_component_key);

            return *std::any_cast<Shared_component_t>(&value);
        }

        template<Concept::Shared_component Shared_component_t>
        void set_shared_component(
            Shared_component_key const shared_component_key,
            Shared_component_t shared_component
        ) noexcept
        {
            m_shared_components[shared_component_key] = shared_component;
        }

        void change_entity_shared_component(Entity const entity, Shared_component_key const shared_component_key) noexcept
        {
            Entity_location_info& location_info = m_entity_location_info[entity.value];
            
            Chunk_group_hash const old_key = location_info.chunk_group_hash;
            Chunk_group_hash const new_key = shared_component_key;

            Component_chunk_group& chunk_group = m_component_chunk_groups[location_info.archetype_index];
            Entity_move_result const result = chunk_group.move_entity(old_key, location_info.chunk_group_index, new_key);

            if (result.entity_moved_by_remove.has_value())
            {
                Entity const moved_entity = result.entity_moved_by_remove->entity;
                
                Entity_location_info& moved_location_info = m_entity_location_info[moved_entity.value];

                Component_chunk_group::Index const new_moved_entity_index = location_info.chunk_group_index;
                moved_location_info.chunk_group_index = new_moved_entity_index;
            }

            location_info.chunk_group_hash = shared_component_key;
            location_info.chunk_group_index = result.new_index;
        }

        /*std::span<Component_chunk_view> get_component_chunk_views(Archetype const archetype) noexcept
        {
            return {};
        }

        std::span<Component_chunk_view const> get_component_chunk_views(Archetype const archetype) const noexcept
        {
            return {};
        }*/

    private:

        Archetype_index add_archetype(Archetype archetype)
        {
            std::span<Component_type_size const> const component_type_sizes = archetype.get_component_type_sizes();
            
            std::size_t const total_component_size_in_bytes = 
                std::accumulate(component_type_sizes.begin(), component_type_sizes.end(), sizeof(Entity));

            constexpr std::size_t maximum_chunk_size_in_bytes = 16 * 1024;
            std::size_t const number_of_entities_per_chunk = maximum_chunk_size_in_bytes / total_component_size_in_bytes;

            m_component_chunk_groups.push_back(
                Component_chunk_group
                {
                    archetype.get_component_type_ids(),
                    component_type_sizes,
                    number_of_entities_per_chunk,
                    m_component_chunks_allocator,
                    m_generic_allocator
                }
            );

            m_archetypes.push_back(std::move(archetype));

            return m_archetypes.size() - 1;
        }

        Archetype_index add_archetype_if_it_does_not_exist(Archetype const& archetype)
        {
            std::optional<Archetype_index> const archetype_index =
                find_archetype_index(archetype);

            if (archetype_index.has_value()) [[likely]]
            {
                return *archetype_index;
            }
            else
            {
                return add_archetype(archetype);
            }
        }

        std::optional<Archetype_index> find_archetype_index(Archetype const& archetype) const noexcept
        {
            auto const archetype_iterator = std::find(m_archetypes.begin(), m_archetypes.end(), archetype);

            if (archetype_iterator != m_archetypes.end())
            {
                return std::distance(m_archetypes.begin(), archetype_iterator);
            }
            else
            {
                return {};
            }
        }

        Entity::Integral_type get_next_entity_value() const noexcept
        {
            return m_next_entity_value;
        }

        void increment_next_entity_value() noexcept
        {
            ++m_next_entity_value;
        }

        Entity create_entity(Archetype_index const archetype_index, Chunk_group_hash const chunk_group_hash)
        {
            Entity::Integral_type const new_entity_value = get_next_entity_value();
            increment_next_entity_value();

            Entity const new_entity{new_entity_value};
  
            Component_chunk_group& component_chunk_group = m_component_chunk_groups[archetype_index];

            Component_chunk_group::Index component_chunk_group_index = 
                component_chunk_group.add_entity(new_entity, chunk_group_hash);

            m_entity_location_info.push_back(
                Entity_location_info
                {
                    archetype_index,
                    chunk_group_hash,
                    component_chunk_group_index
                }
            );
            
            return new_entity;
        }

        void move_entity(Entity_location_info const& entity_location_info, Component_chunk_group& to)
        {
            Component_chunk_group& from = m_component_chunk_groups[entity_location_info.archetype_index];

            // TODO

            // Create new entity in to component chunk group

            // For each old component, read from old chunk and write to new chunk

            // Remove entity from from component chunk group
        }

        std::pmr::polymorphic_allocator<std::byte> m_generic_allocator;
        std::pmr::polymorphic_allocator<std::byte> m_component_chunks_allocator;
        
        std::pmr::vector<Archetype> m_archetypes;
        std::pmr::vector<Component_chunk_group> m_component_chunk_groups;

        std::pmr::vector<Entity_location_info> m_entity_location_info;

        std::pmr::unordered_map<Chunk_group_hash, std::any> m_shared_components;
        
        Entity::Integral_type m_next_entity_value;
    };
}
