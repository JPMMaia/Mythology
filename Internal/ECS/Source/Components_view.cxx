export module maia.ecs.components_view;

import maia.ecs.component;
import maia.ecs.entity;
import maia.ecs.shared_component;

import <cstddef>;
import <iterator>;
import <span>;

namespace Maia::ECS
{
    export class Entity_components_view
    {
    public:

        template<Concept::Component Component_t>
        Component_t get() const noexcept
        {
            return {};
        }

        template<Concept::Component Component_t>
        void set(Component_t const value) const noexcept
        {
        }
    };

    export class Const_entity_components_view
    {
    public:

        template<Concept::Component Component_t>
        Component_t get() const noexcept
        {
            return {};
        }
    };

    export class Component_chunk_view_iterator
    {
    public:

        using difference_type =	std::ptrdiff_t;
        using value_type = std::remove_cv_t<Entity_components_view>;
        using pointer =	Entity_components_view*;
        using reference = Entity_components_view&;
        using iterator_category = std::random_access_iterator_tag;

        reference operator*() const noexcept
        {
            return *m_view;
        }

        pointer operator->() const noexcept
        {
            return m_view;
        }

        Component_chunk_view_iterator& operator++() noexcept
        {
            // TODO increment
            return *this;
        }

        Component_chunk_view_iterator operator++(int) noexcept
        {
            // TODO increment
            return *this;
        }

        Component_chunk_view_iterator& operator--() noexcept
        {
            // TODO decrement
            return *this;
        }

        Component_chunk_view_iterator operator--(int) noexcept
        {
            // TODO decrement
            return *this;
        }

        Component_chunk_view_iterator& operator+=(difference_type value) noexcept
        {
            // TODO
            return *this;
        }

        Component_chunk_view_iterator operator+(difference_type value) const noexcept
        {
            // TODO
            return *this;
        }

        Component_chunk_view_iterator& operator-=(difference_type value) noexcept
        {
            // TODO
            return *this;
        }

        Component_chunk_view_iterator operator-(difference_type value) const noexcept
        {
            // TODO
            return *this;
        }

        difference_type operator-(Component_chunk_view_iterator const value) const noexcept
        {
            // TODO
            return 0;
        }

        reference operator[](std::size_t const value) const noexcept
        {
            // TODO
            return *m_view;
        }

        bool operator<(Component_chunk_view_iterator const view) const noexcept
        {
            // TODO
            return true;
        }

        bool operator>(Component_chunk_view_iterator const view) const noexcept
        {
            // TODO
            return true;
        }

        bool operator>=(Component_chunk_view_iterator const view) const noexcept
        {
            // TODO
            return true;
        }

        bool operator<=(Component_chunk_view_iterator const view) const noexcept
        {
            // TODO
            return true;
        }

    private:

        Entity_components_view* m_view;
    };

    export bool operator==(Component_chunk_view_iterator const& lhs, Component_chunk_view_iterator const& rhs) noexcept
    {
        return true;
    }

    export bool operator!=(Component_chunk_view_iterator const& lhs, Component_chunk_view_iterator const& rhs) noexcept
    {
        return !(lhs == rhs);
    }

    export class Component_chunk_view
    {
    public:

        template<Concept::Shared_component Shared_component_t>
        Shared_component_t const& get_shared_component() const noexcept
        {
            static Shared_component_t dummy;
            return dummy;
        }

        std::span<Entity const> get_entities() const noexcept
        {
            return {};
        }

        std::size_t get_entity_count() const noexcept
        {
            return {};
        }

        Entity_components_view get_entity_components_view(std::size_t const index) noexcept
        {
            return {};
        }

        Const_entity_components_view get_entity_components_view(std::size_t const index) const noexcept
        {
            return {};
        }

        Component_chunk_view_iterator begin() const noexcept
        {
            return {};
        }

        Component_chunk_view_iterator end() const noexcept
        {
            return {};
        }
    };
}
