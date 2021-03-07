module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>

export module maia.ecs.hash_map;

namespace Maia::ECS
{
    export template <
        typename Key_t,
        typename Value_t,
        typename Hash_t = std::hash<Key_t>,
        typename Key_equal_t = std::equal_to<Key_t>,
        typename Allocator_t = std::allocator<std::pair<Key_t, Value_t>>
    >
    class Hash_map
    {
    public:

        using Iterator = std::pair<Key_t, Value_t>*; //TODO key should be const somehow..
        using Const_iterator = std::pair<Key_t, Value_t> const*;

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

        ~Hash_map() noexcept
        {
            // TODO
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
            return size() == 0;
        }

        std::size_t size() noexcept
        {
            return m_count;
        }

        template <typename... Arguments>
        std::pair<Iterator, bool> emplace(Arguments&&... arguments)
        {
            // TODO should we rename this?

            std::size_t const next_count = std::max<std::size_t>(m_count + 1, 8);

            if (should_resize(next_count, m_capacity))
            {
                reserve(2 * next_count);
            }

            std::pair<Key_t, Value_t> new_pair = std::make_pair(std::forward<Arguments>(arguments)...);
            Key_t const& new_key = new_pair.first;
            
            std::size_t index = calculate_index_hint(new_key);

            while (is_content_valid(index) && !is_same_key(index, new_key))
            {
                index = (index + 1) % m_capacity;
            }

            m_content[index] = std::move(new_pair);
            set_content_valid(index, true);

            m_count++;

            return {};
        }

        Iterator begin() noexcept
        {
            return m_content;
        }

        Const_iterator begin() const noexcept
        {
            return m_content;
        }

        Iterator end() noexcept
        {
            return m_content + m_capacity;
        }

        Const_iterator end() const noexcept
        {
            return m_content + m_capacity;
        }

        Iterator find(Key_t const& key) noexcept
        {
            std::size_t index = calculate_index_hint(key);

            while (is_content_valid(index) && !is_same_key(index, key))
            {
                index = (index + 1) % m_capacity;
            }

            if (is_content_valid(index))
            {
                return m_content + index;
            }
            else
            {
                return end();
            }
        }

        Const_iterator find(Key_t const& key) const noexcept
        {
            std::size_t index = calculate_index_hint(key);

            while (is_content_valid(index) && !is_same_key(index, key))
            {
                index = (index + 1) % m_capacity;
            }

            if (is_content_valid(index))
            {
                return m_content + index;
            }
            else
            {
                return end();
            }
        }

        Value_t& at(Key_t const& key)
        {
            Iterator const iterator = find(key);

            if (iterator != end())
            {
                return iterator->second;
            }
            else
            {
                // TODO test
                throw std::out_of_range{"Key was not found!"};
            }
        }

        Value_t const& at(Key_t const& key) const
        {
            Const_iterator const iterator = find(key);

            if (iterator != end())
            {
                return iterator->second;
            }
            else
            {
                // TODO test
                throw std::out_of_range{"Key was not found!"};
            }
        }

        bool contains(Key_t const& key) const noexcept
        {
            Const_iterator const iterator = find(key);

            return iterator != end();
        }

        void erase(Key_t const& key) noexcept
        {
            Iterator const iterator = find(key);

            if (iterator != end())
            {
                std::size_t const index = std::distance(begin(), iterator);
                set_content_valid(index, false);
            }
        }

        void clear() noexcept
        {
            m_count = 0;
            std::memset(m_is_content_valid, 0, m_capacity / 8);
        }

        void swap(Hash_map& other) noexcept
        {
        }

        void reserve(std::size_t const count)
        {
            constexpr std::size_t load_factor = 70;
            std::size_t const new_capacity = count * 100 / load_factor;

            {
                std::pair<Key_t, Value_t>* const new_content = m_allocator.allocate(new_capacity);
                
                /*for (std::size_t index = 0; index < m_count; ++index)
                {
                    if (is_content_valid(index))
                    {
                        new_content[index] = m_content[index];
                    }
                }*/

                m_allocator.deallocate(m_content, m_capacity);
                m_content = new_content;
            }

            /*std::allocator_traits<decltype(m_allocator)>::rebind_alloc<std::byte> bytes_allocator;
            std::byte* new_is_content_valid = bytes_allocator.allocate(new_capacity);*/
            
            {
                using Byte_allocator = std::allocator_traits<decltype(m_allocator)>::template rebind_alloc<std::byte>;
                
                Byte_allocator bytes_allocator;

                constexpr std::size_t number_of_bits_in_a_byte = 8;

                std::size_t const number_of_bytes_to_allocate = new_capacity / number_of_bits_in_a_byte;
                std::byte* const new_is_content_valid = bytes_allocator.allocate(number_of_bytes_to_allocate);

                /*
                // TODO
                */

                std::memset(new_is_content_valid, 0, new_capacity * sizeof(std::byte));
                
                std::size_t const number_of_bytes_to_deallocate = m_capacity / number_of_bits_in_a_byte;
                bytes_allocator.deallocate(m_is_content_valid, number_of_bytes_to_deallocate);
                m_is_content_valid = new_is_content_valid;
            }

            m_capacity = new_capacity;
        }

        static constexpr std::size_t get_required_memory_size(std::size_t const count) noexcept
        {
            return 1;
        }

    private:

        static bool should_resize(std::size_t const count, std::size_t const capacity) noexcept
        {
            if (count > capacity)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        bool is_content_valid(std::size_t const index) const noexcept
        {
            assert(index < m_capacity);

            constexpr std::size_t number_of_bits_in_a_byte = 8;

            std::size_t const mask_index = index / number_of_bits_in_a_byte;
            std::byte const mask = m_is_content_valid[mask_index];

            std::size_t const bit_index = index % number_of_bits_in_a_byte;
            std::byte const bit_mask = std::byte{1} << bit_index;
            return (mask & bit_mask) != std::byte{0};
        }

        void set_content_valid(std::size_t const index, bool valid) noexcept
        {
            assert(index < m_capacity);

            constexpr std::size_t number_of_bits_in_a_byte = 8;

            std::size_t const mask_index = index / number_of_bits_in_a_byte;
            std::size_t const bit_index = index % number_of_bits_in_a_byte;

            if (valid)
            {
                std::byte const bit_mask = std::byte{1} << bit_index;
                m_is_content_valid[mask_index] |= bit_mask;
            }
            else
            {
                std::byte const bit_mask = ~(std::byte{1} << bit_index);
                m_is_content_valid[mask_index] &= bit_mask;
            }
        }

        bool is_same_key(std::size_t const index, Key_t const& key) const noexcept
        {
            assert(index < m_capacity);

            std::pair<Key_t, Value_t> const& pair = m_content[index];

            return Key_equal_t{}(pair.first, key);
        }

        std::size_t calculate_index_hint(Key_t const& key) const noexcept
        {            
            std::size_t const hash_value = Hash_t{}(key);
            
            return hash_value % m_capacity;
        }

    private:

        std::pair<Key_t, Value_t>* m_content = nullptr;
        std::byte* m_is_content_valid = nullptr;
        std::size_t m_capacity = 0;
        std::size_t m_count = 0;
        Allocator_t m_allocator;

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
            std::pmr::polymorphic_allocator<std::pair<Key_t, Value_t>>
        >;
    }
}
