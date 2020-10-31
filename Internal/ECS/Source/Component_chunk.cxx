export module maia.ecs.component_chunk;

import maia.ecs.component;
import maia.ecs.entity;

import <cassert>;
import <cstddef>;
import <cstring>;
import <memory_resource>;
import <span>;
import <vector>;

namespace Maia::ECS
{
    export class Component_chunk
    {
    public:

        using Size_type = std::size_t;

        Component_chunk(
            std::span<Component_type_info const> const component_type_infos,
            Size_type const num_components_of_each_type,
            std::pmr::polymorphic_allocator<std::byte> const& allocator
        ) :
            m_components_data{allocator},
            m_begin_offsets{allocator},
            m_component_type_infos{component_type_infos.begin(), component_type_infos.end(), allocator},
            m_size{num_components_of_each_type}
        {
            m_begin_offsets.resize(component_type_infos.size(), 0);

            Size_type begin_offset = 0;

            for (std::size_t type_index = 0; type_index < component_type_infos.size(); ++type_index)
            {
                m_begin_offsets[type_index] = begin_offset;
                begin_offset += component_type_infos[type_index].size;
            }

            Size_type total_size = 0;

            for (std::size_t type_index = 0; type_index < component_type_infos.size(); ++type_index)
            {
                total_size += component_type_infos[type_index].size;
            }

            total_size *= num_components_of_each_type;

            m_components_data.resize(total_size, std::byte{});
        }

        Size_type size() const noexcept
        {
            return m_size;
        }

        template<Concept::Component Component_t>
        Component_t get_component_value(Size_type const index) const noexcept
        {
            assert(index < size());

            Component_type_ID const component_type_id = get_component_type_id<Component_t>();
            assert(has_component_type(component_type_id));

            auto const is_component_type = [component_type_id](Component_type_info const type_info) -> bool
            {
                return type_info.id == component_type_id;
            };

            auto const type_info_location = std::find_if(
                m_component_type_infos.begin(),
                m_component_type_infos.end(),
                is_component_type
            );
            auto const type_index = std::distance(m_component_type_infos.begin(), type_info_location);
            Size_type const type_begin_offset = m_begin_offsets[type_index];
            assert(type_info_location->size == sizeof(Component_t));

            Component_t value = {};
            std::byte const* const source = m_components_data.data() + type_begin_offset + index * sizeof(Component_t);
            std::memcpy(&value, source, sizeof(Component_t));

            return value;
        }

        template<Concept::Component Component_t>
        void set_component_value(Size_type const index, Component_t const value) noexcept
        {
            assert(index < size());

            Component_type_ID const component_type_id = get_component_type_id<Component_t>();
            assert(has_component_type(component_type_id));

            auto const is_component_type = [component_type_id](Component_type_info const type_info) -> bool
            {
                return type_info.id == component_type_id;
            };

            auto const type_info_location = std::find_if(
                m_component_type_infos.begin(),
                m_component_type_infos.end(),
                is_component_type
            );
            auto const type_index = std::distance(m_component_type_infos.begin(), type_info_location);
            Size_type const type_begin_offset = m_begin_offsets[type_index];
            assert(type_info_location->size == sizeof(Component_t));

            std::byte* const destination = m_components_data.data() + type_begin_offset + index * sizeof(Component_t);
            std::memcpy(destination, &value, sizeof(Component_t));
        }

        bool has_component_type(Component_type_ID const component_type_id) const noexcept
        {
            auto const is_component_type = [component_type_id](Component_type_info const type_info) -> bool
            {
                return type_info.id == component_type_id;
            };

            auto const location = std::find_if(
                m_component_type_infos.begin(),
                m_component_type_infos.end(),
                is_component_type
            );

            return location != m_component_type_infos.end();
        }

    private:

        std::pmr::vector<std::byte> m_components_data;
        std::pmr::vector<Size_type> m_begin_offsets;
        std::pmr::vector<Component_type_info> m_component_type_infos;
        Size_type m_size;

    };
}
