module;

#include <cstddef>
#include <functional>
#include <memory>
#include <utility>

export module maia.ecs.hash_map;

namespace Maia::ECS
{
    export template <
        typename Key_t,
        typename Value_t,
        typename Hash_t = std::hash<Key_t>,
        typename Key_equal_t = std::equal_to<Key_t>,
        typename Allocator_t = std::allocator<std::pair<Key_t const, Value_t>>
    >
    class Hash_map
    {
    public:

        using Iterator = std::pair<Key_t const, Value_t>*;
        using Const_iterator = std::pair<Key_t const, Value_t> const*;

        Hash_map() noexcept = default;

        Hash_map(Hash_map const& other)
        {
        }

        Hash_map(Hash_map&& other) noexcept
        {
        }

        explicit Hash_map(Allocator_t const& allocator) noexcept
        {
        }

        Hash_map& operator=(Hash_map const& other)
        {
            return *this;
        }

        Hash_map& operator=(Hash_map&& other) noexcept
        {
            return *this;
        }

        bool empty() noexcept
        {
            return true;
        }

        std::size_t size() noexcept
        {
            return 0;
        }

        template <typename... Arguments>
        std::pair<Iterator, bool> emplace(Arguments&&... arguments)
        {
            return {};
        }

        Iterator begin() noexcept
        {
            return {};
        }

        Const_iterator begin() const noexcept
        {
            return {};
        }

        Iterator end() noexcept
        {
            return {};
        }

        Const_iterator end() const noexcept
        {
            return {};
        }

        Iterator find(Key_t const& key) noexcept
        {
            return {};
        }

        Const_iterator find(Key_t const& key) const noexcept
        {
            return {};
        }

        Value_t& at(Key_t const& key)
        {
            static Value_t dummy;
            return dummy;
        }

        Value_t const& at(Key_t const& key) const
        {
            static Value_t dummy;
            return dummy;
        }

        bool contains(Key_t const& key) const noexcept
        {
            return false;
        }

        void erase(Key_t const& key) noexcept
        {
        }

        void clear() noexcept
        {
        }

        void swap(Hash_map& other) noexcept
        {
        }

        void reserve(std::size_t const count)
        {
        }

        static constexpr std::size_t get_required_memory_size(std::size_t const count) noexcept
        {
            return 1;
        }
    };

    namespace pmr
    {
        export template <
            typename Key_t,
            typename Value_t,
            typename Hash_t = std::hash<Key_t>,
            typename Key_equal_t = std::equal_to<Key_t>
        >
        using Hash_map = Maia::ECS::Hash_map<
            Key_t,
            Value_t,
            Hash_t,
            Key_equal_t,
            std::pmr::polymorphic_allocator<std::pair<const Key_t, Value_t>>
        >;
    }
}
