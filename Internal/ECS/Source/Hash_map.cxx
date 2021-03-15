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
    namespace detail
    {
        bool is_content_valid(std::byte const* const is_content_valid, std::size_t const index) noexcept
        {
            constexpr std::size_t number_of_bits_in_a_byte = 8;

            std::size_t const mask_index = index / number_of_bits_in_a_byte;
            std::byte const mask = is_content_valid[mask_index];

            std::size_t const bit_index = index % number_of_bits_in_a_byte;
            std::byte const bit_mask = std::byte{1} << bit_index;
            return (mask & bit_mask) != std::byte{0};
        }
    }

    export template <
        typename Key_t,
        typename Value_t,
        bool is_const_t
    >
    class Hash_map_iterator
    {
    public:
        
        using difference_type = std::ptrdiff_t;
        using value_type = std::pair<Key_t const, Value_t>;
        using pointer = std::conditional_t<is_const_t, std::pair<Key_t const, Value_t> const*, std::pair<Key_t const, Value_t>*>;
        using reference = std::conditional_t<is_const_t, std::pair<Key_t const, Value_t> const&, std::pair<Key_t const, Value_t>&>;
        using iterator_category = std::bidirectional_iterator_tag;
        using iterator_concept = std::bidirectional_iterator_tag;

        using Content_pointer = std::conditional_t<is_const_t, std::pair<Key_t const, Value_t> const*, std::pair<Key_t const, Value_t>*>;
        using Is_valid_pointer = std::conditional_t<is_const_t, std::byte const*, std::byte*>;

        Hash_map_iterator(Content_pointer const content, Is_valid_pointer const is_valid, std::size_t const capacity, std::size_t const index) noexcept :
            m_content{content},
            m_is_valid{is_valid},
            m_index{index},
            m_capacity{capacity}
        {
        }

        std::strong_ordering operator<=>(Hash_map_iterator const& other) const noexcept
        {
            if (auto cmp = (m_content <=> other.m_content); cmp != 0)
            {
                return cmp;
            }
            else
            {
                assert(m_is_valid == other.m_is_valid);
                assert(m_capacity == other.m_capacity);

                return m_index <=> other.m_index;
            }
        }

        bool operator==(Hash_map_iterator const& other) const noexcept
        {
            if (m_content != other.m_content)
            {
                return false;
            }
            else
            {
                assert(m_is_valid == other.m_is_valid);
                assert(m_capacity == other.m_capacity);

                return m_index == other.m_index;
            }
        }

        bool operator!=(Hash_map_iterator const& other) const noexcept
        {
            return !(*this == other);
        }

        reference operator*() const noexcept
        {
            return *(m_content + m_index);
        }

        pointer operator->() const noexcept
        {
            return (m_content + m_index);
        }

        Hash_map_iterator& operator++() noexcept
        {
            std::size_t next_index = m_index;
            ++next_index;

            while (next_index < m_capacity && !detail::is_content_valid(m_is_valid, next_index))
            {
                ++next_index;
            }

            m_index = next_index;

            return *this;
        }

        Hash_map_iterator operator++(int) noexcept
        {
            // TODO test
            Hash_map_iterator const copy = *this;
            ++(*this);
            return copy;
        }

        Hash_map_iterator& operator--() noexcept
        {
            // TODO test
            std::size_t previous_index = m_index;
            ++previous_index;

            while (previous_index > 0 && !detail::is_content_valid(m_is_valid, previous_index - 1))
            {
                --previous_index;
            }

            m_index = previous_index;

            return *this;
        }

        Hash_map_iterator operator--(int) noexcept
        {
            // TODO test
            Hash_map_iterator const copy = *this;
            --(*this);
            return copy;
        }

    private:

        Content_pointer m_content;
        Is_valid_pointer m_is_valid;
        std::size_t m_index;
        std::size_t m_capacity;

        template <
            typename Key_t,
            typename Value_t,
            typename Hash_t,
            typename Key_equal_t,
            typename Allocator_t
        >
        friend class Hash_map;

    };

    constexpr std::size_t c_minimum_number_of_elements = 16;
    static_assert((c_minimum_number_of_elements % 2 == 0));

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

        using Iterator = Hash_map_iterator<Key_t, Value_t, false>;
        using Const_iterator = Hash_map_iterator<Key_t, Value_t, true>;

        Hash_map() noexcept = default;

        Hash_map(Hash_map const& other) :
            m_content{nullptr},
            m_is_content_valid{nullptr},
            m_capacity{0},
            m_count{0},
            m_allocator{other.m_allocator}
        {
            reserve(other.m_count);

            for (std::pair<Key_t const, Value_t> const& value : other)
            {
                insert_or_assign(value);
            }
        }

        Hash_map(Hash_map&& other) noexcept :
            m_content{std::exchange(other.m_content, nullptr)},
            m_is_content_valid{std::exchange(other.m_is_content_valid, nullptr)},
            m_capacity{std::exchange(other.m_capacity, 0)},
            m_count{std::exchange(other.m_count, 0)},
            m_allocator{other.m_allocator}
        {
        }

        explicit Hash_map(Allocator_t const& allocator) noexcept :
            m_content{nullptr},
            m_is_content_valid{nullptr},
            m_capacity{0},
            m_count{0},
            m_allocator{allocator}
        {
        }

        ~Hash_map() noexcept
        {
            clear();

            if (m_content != nullptr)
            {
                m_allocator.deallocate(m_content, m_capacity);
            }

            if (m_is_content_valid != nullptr)
            {
                using Byte_allocator = std::allocator_traits<decltype(m_allocator)>::template rebind_alloc<std::byte>;
                
                Byte_allocator bytes_allocator{m_allocator};

                constexpr std::size_t number_of_bits_in_a_byte = 8;
                std::size_t const number_of_bytes_to_deallocate = (m_capacity / number_of_bits_in_a_byte) + ((m_capacity % number_of_bits_in_a_byte) != 0 ? 1 : 0);
                bytes_allocator.deallocate(m_is_content_valid, number_of_bytes_to_deallocate);
            }
        }

        Hash_map& operator=(Hash_map const& other)
        {
            clear();
            reserve(other.m_count);

            for (std::pair<Key_t const, Value_t> const& value : other)
            {
                insert_or_assign(value);
            }

            return *this;
        }

        Hash_map& operator=(Hash_map&& other) noexcept
        {
            m_content = std::exchange(other.m_content, nullptr);
            m_is_content_valid = std::exchange(other.m_is_content_valid, nullptr);
            m_capacity = std::exchange(other.m_capacity, 0);
            m_count = std::exchange(other.m_count, 0);
            m_allocator = other.m_allocator;

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

        std::pair<Iterator, bool> insert_or_assign(std::pair<Key_t const, Value_t> const& value)
        {
            std::pair<Key_t const, Value_t> copy = value;
            return insert_or_assign(std::move(copy));
        }

        std::pair<Iterator, bool> insert_or_assign(std::pair<Key_t const, Value_t>&& value)
        {
            if (should_resize(m_count + 1, m_capacity))
            {
                reserve(2 * m_count);
            }

            Key_t const& new_key = value.first;
            
            std::size_t index = calculate_index_hint(new_key);

            while (is_content_valid(index) && !is_same_key(index, new_key))
            {
                index = (index + 1) % m_capacity;
            }

            {
                std::pair<Key_t const, Value_t>* location = &m_content[index];
                std::construct_at(location, std::move(value));
            }

            bool const inserted = !is_content_valid(index);
            set_content_valid(index, true);

            m_count++;

            return
            {
                Iterator{m_content, m_is_content_valid, m_capacity, index},
                inserted
            };
        }

        Iterator begin() noexcept
        {
            std::size_t const first_valid_index = get_first_valid_index();

            return Iterator{m_content, m_is_content_valid, m_capacity, first_valid_index};
        }

        Const_iterator begin() const noexcept
        {
            std::size_t const first_valid_index = get_first_valid_index();

            return Const_iterator{m_content, m_is_content_valid, m_capacity, first_valid_index};
        }

        Iterator end() noexcept
        {
            return Iterator{m_content, m_is_content_valid, m_capacity, m_capacity};
        }

        Const_iterator end() const noexcept
        {
            return Const_iterator{m_content, m_is_content_valid, m_capacity, m_capacity};
        }

        Iterator find(Key_t const& key) noexcept
        {
            if (m_count == 0)
            {
                return end();
            }

            std::size_t index = calculate_index_hint(key);

            while (is_content_valid(index) && !is_same_key(index, key))
            {
                index = (index + 1) % m_capacity;
            }

            if (is_content_valid(index))
            {
                return Iterator{m_content, m_is_content_valid, m_capacity, index};
            }
            else
            {
                return end();
            }
        }

        Const_iterator find(Key_t const& key) const noexcept
        {
            if (m_count == 0)
            {
                return end();
            }

            std::size_t index = calculate_index_hint(key);

            while (is_content_valid(index) && !is_same_key(index, key))
            {
                index = (index + 1) % m_capacity;
            }

            if (is_content_valid(index))
            {
                return Const_iterator{m_content, m_is_content_valid, m_capacity, index};
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
                {
                    std::size_t const index = iterator.m_index;
                    // TODO std::destroy_at test
                    std::destroy_at(&m_content[index]);
                }
                {
                    std::size_t index = iterator.m_index;
                    
                    for (std::size_t j = (index + 1) % m_capacity; is_content_valid(j); j = (j + 1) % m_capacity)
                    {
                        std::size_t const k = calculate_index_hint(m_content[j].first);

                        if (j > index && (k <= index || k > j) ||
                           (j < index && (k <= index && k >= j)))
                        {
                            std::construct_at(&m_content[index], std::move(m_content[j]));
                            std::destroy_at(&m_content[j]);
                            index = j;
                        }
                    }

                    set_content_valid(index, false);
                }

                --m_count;
            }
        }

        void clear() noexcept
        {
            for (auto iterator = begin(); iterator != end(); ++iterator)
            {
                std::destroy_at(&(*iterator));
            }

            m_count = 0;
            std::memset(m_is_content_valid, 0, m_capacity / 8);
        }

        void swap(Hash_map& other) noexcept
        {
            std::swap(m_content, other.m_content);
            std::swap(m_is_content_valid, other.m_is_content_valid);
            std::swap(m_capacity, other.m_capacity);
            std::swap(m_count, other.m_count);
            std::swap(m_allocator, other.m_allocator);
        }

        void reserve(std::size_t const count)
        {
            std::size_t const old_capacity = m_capacity;
            std::size_t const new_capacity = calculate_capacity(count);

            if (old_capacity >= new_capacity)
            {
                return;
            }
            
            m_capacity = new_capacity;

            {
                using Byte_allocator = std::allocator_traits<decltype(m_allocator)>::template rebind_alloc<std::byte>;
                
                Byte_allocator bytes_allocator{m_allocator};

                constexpr std::size_t number_of_bits_in_a_byte = 8;

                std::size_t const number_of_bytes_to_allocate = (new_capacity / number_of_bits_in_a_byte) + ((new_capacity % number_of_bits_in_a_byte) != 0 ? 1 : 0);
                std::byte* const new_is_content_valid = bytes_allocator.allocate(number_of_bytes_to_allocate);

                std::memset(new_is_content_valid, 0, number_of_bytes_to_allocate);
                
                if (m_is_content_valid != nullptr)
                {
                    std::size_t const number_of_bytes_to_deallocate = (old_capacity / number_of_bits_in_a_byte) + ((old_capacity % number_of_bits_in_a_byte) != 0 ? 1 : 0);
                    
                    std::memcpy(new_is_content_valid, m_is_content_valid, number_of_bytes_to_deallocate);
                    
                    bytes_allocator.deallocate(m_is_content_valid, number_of_bytes_to_deallocate);
                }
                
                m_is_content_valid = new_is_content_valid;
            }

            {
                std::pair<Key_t const, Value_t>* const old_content = m_content;
                m_content = m_allocator.allocate(new_capacity);

                if (old_content != nullptr)
                {
                    m_count = 0;

                    {
                        Iterator const old_content_begin{old_content, m_is_content_valid, old_capacity, get_first_valid_index(m_is_content_valid, old_capacity)};
                        Iterator const old_content_end{old_content, m_is_content_valid, old_capacity, old_capacity};
                        
                        for (auto iterator = old_content_begin; iterator != old_content_end; ++iterator)
                        {
                            insert_or_assign(std::move(*iterator));
                            std::destroy_at(&(*iterator));
                        }
                    }

                    m_allocator.deallocate(old_content, old_capacity);
                }
            }
        }

        static constexpr std::size_t get_required_memory_size(std::size_t const count) noexcept
        {
            std::size_t const required_capacity = calculate_capacity(count);

            return required_capacity * sizeof(std::pair<Key_t const, Value_t>) + required_capacity / 8 + ((required_capacity % 8 != 0) ? 1 : 0);
            // TODO test std::max(c_minimum_number_of_elements, count)
        }

    private:

        static bool should_resize(std::size_t const count, std::size_t const capacity) noexcept
        {
            if (calculate_capacity(count) > capacity)
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

            return detail::is_content_valid(m_is_content_valid, index);
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

        std::size_t get_first_valid_index() const noexcept
        {
            return get_first_valid_index(m_is_content_valid, m_capacity);
        }

        static std::size_t get_first_valid_index(std::byte const* const is_content_valid, std::size_t const capacity) noexcept
        {
            for (std::size_t index = 0; index < capacity; ++index)
            {
                if (detail::is_content_valid(is_content_valid, index))
                {
                    return index;
                }
            }

            return capacity;
        }

        bool is_same_key(std::size_t const index, Key_t const& key) const noexcept
        {
            assert(index < m_capacity);

            std::pair<Key_t const, Value_t> const& pair = m_content[index];

            return Key_equal_t{}(pair.first, key);
        }

        std::size_t calculate_index_hint(Key_t const& key) const noexcept
        {
            assert(m_capacity != 0);

            std::size_t const hash_value = Hash_t{}(key);
            
            return hash_value % m_capacity;
        }

        static constexpr std::size_t calculate_capacity(std::size_t count) noexcept
        {
            constexpr std::size_t load_factor = 70;
            return std::max<std::size_t>(count, c_minimum_number_of_elements) * 100 / load_factor;
        }

    private:

        std::pair<Key_t const, Value_t>* m_content = nullptr;
        std::byte* m_is_content_valid = nullptr;
        std::size_t m_capacity = 0;
        std::size_t m_count = 0;
        Allocator_t m_allocator;
        // TODO custom equal and hash

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
            std::pmr::polymorphic_allocator<std::pair<Key_t const, Value_t>>
        >;
    }
}
