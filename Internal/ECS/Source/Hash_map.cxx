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

        template <class Allocator, class Value_type = typename std::allocator_traits<Allocator>::value_type>
        auto create_unique_ptr(Value_type* const pointer, Allocator& allocator, std::size_t const size) noexcept
        {
            auto const deallocate = [&allocator, size](Value_type* const pointer) -> void
            {
                allocator.deallocate(pointer, size);
            };

            return std::unique_ptr<Value_type, decltype(deallocate)>
            {
                pointer,
                deallocate
            };
        }

        template <class Allocator>
        auto create_unique_ptr(Allocator& allocator, std::size_t const size)
        {
            return create_unique_ptr(
                allocator.allocate(size),
                allocator,
                size
            );
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

        Hash_map_iterator() noexcept = default;

        Hash_map_iterator(Content_pointer const content, Is_valid_pointer const is_valid, std::size_t const capacity, std::size_t const index) noexcept :
            m_content{content},
            m_is_valid{is_valid},
            m_index{index},
            m_capacity{capacity}
        {
        }

        template <typename = typename std::enable_if<is_const_t>>
        Hash_map_iterator(Hash_map_iterator<Key_t, Value_t, false> const& other) noexcept :
            m_content{other.m_content},
            m_is_valid{other.m_is_valid},
            m_index{other.m_index},
            m_capacity{other.m_capacity}
        {
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
            Hash_map_iterator const copy = *this;
            ++(*this);
            return copy;
        }

        Hash_map_iterator& operator--() noexcept
        {
            std::size_t previous_index = m_index;
            --previous_index;

            while (previous_index > 0 && !detail::is_content_valid(m_is_valid, previous_index))
            {
                --previous_index;
            }

            m_index = previous_index;

            return *this;
        }

        Hash_map_iterator operator--(int) noexcept
        {
            Hash_map_iterator const copy = *this;
            --(*this);
            return copy;
        }

    private:

        Content_pointer m_content = nullptr;
        Is_valid_pointer m_is_valid = nullptr;
        std::size_t m_index = 0;
        std::size_t m_capacity = 0;

        friend Hash_map_iterator<Key_t, Value_t, true>;

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

    /**
     * @brief A container that maps keys to values.
     * 
     * The method used to resolve hash collisions is open addressing with
     * linear probing.
     * 
     * It supports custom hash functions, key comparisions and allocators.
     * 
     * It's possible to allocate all dynamic memory beforehand by using the
     * #reserve method. To get the number of bytes needed for the allocation,
     * use the static constexpr method #get_required_memory_size.
     */
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

        using value_type = std::pair<Key_t const, Value_t>;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using hasher = Hash_t;
        using key_equal = Key_equal_t;
        using allocator_type = Allocator_t;
        using reference = value_type&;
        using const_reference = value_type const&;
        using pointer = std::allocator_traits<allocator_type>::pointer;
        using const_pointer = std::allocator_traits<allocator_type>::const_pointer;
        using iterator = Hash_map_iterator<Key_t, Value_t, false>;
        using const_iterator = Hash_map_iterator<Key_t, Value_t, true>;

        /**
         * @brief Default constructor.
         * 
         * Create an empty hash map.
         * 
         */
        Hash_map() noexcept = default;

        /**
         * @brief Copy constructor.
         * 
         * Create a new hash map and copy the content of @other.
         * 
         * @param other is the hash map to copy.
         */
        Hash_map(Hash_map const& other) :
            m_content{nullptr},
            m_is_content_valid{nullptr},
            m_capacity{0},
            m_count{0},
            m_allocator{other.m_allocator},
            m_hash{other.m_hash},
            m_key_equal{other.m_key_equal}
        {
            reserve(other.m_count);

            for (std::pair<Key_t const, Value_t> const& keyValue : other)
            {
                insert_or_assign(keyValue.first, keyValue.second);
            }
        }

        /**
         * @brief Move constructor.
         * 
         * Create a new hash map with the same content of @other.
         * It uses the same memory as @other.
         * After this operation, @other becomes an empty object.
         * 
         * @param other is the hash map to move.
         */
        Hash_map(Hash_map&& other) noexcept :
            m_content{std::exchange(other.m_content, nullptr)},
            m_is_content_valid{std::exchange(other.m_is_content_valid, nullptr)},
            m_capacity{std::exchange(other.m_capacity, 0)},
            m_count{std::exchange(other.m_count, 0)},
            m_allocator{std::allocator_traits<Allocator_t>::select_on_container_copy_construction(other.m_allocator)},
            m_hash{other.m_hash},
            m_key_equal{other.m_key_equal}
        {
        }

        /**
         * @brief Construct a new hash map using a custom allocator.
         * 
         * @param allocator is the allocator that will be used to allocate all
         * dynamic memory.
         */
        explicit Hash_map(Allocator_t const& allocator) noexcept :
            Hash_map(Hash_t{}, Key_equal_t{}, allocator)
        {
        }

        /**
         * @brief Construct a hash map using a custom hash function, key
         * comparison and allocator.
         * 
         * @param hash is a function that accepts a key and computes a hash.
         * @param key_equal is a function that compares two keys.
         * @param allocator is the allocator that will be used to allocate all
         * dynamic memory.
         */
        Hash_map(
            Hash_t hash,
            Key_equal_t key_equal,
            Allocator_t const& allocator = Allocator_t{}
        ) noexcept :
            m_content{nullptr},
            m_is_content_valid{nullptr},
            m_capacity{0},
            m_count{0},
            m_allocator{allocator},
            m_hash{hash},
            m_key_equal{key_equal}
        {         
        }

        /**
         * @brief Destroy the hash map object.
         * 
         * Destroy all objects and deallocate all dynamic memory.
         * 
         */
        ~Hash_map() noexcept
        {
            clear_and_deallocate_memory();
        }

        /**
         * @brief Copy assignment.
         * 
         * Destroy all keys and values. If
         * std::allocator_traits<Allocator_t>::propagate_on_container_copy_assignment
         * is true and allocators are different, then deallocate all
         * memory and copy @other allocator. Then reserve memory and copy all
         * content from @other.
         * 
         * @param other is the hash map to copy.
         * @return Hash_map& is a reference to itself.
         */
        Hash_map& operator=(Hash_map const& other)
        {
            // TODO test all objects are destructed
            // TODO test all possibilities

            using Propagate_on_container_copy_assignment =
                typename std::allocator_traits<Allocator_t>::propagate_on_container_copy_assignment;

            if constexpr (Propagate_on_container_copy_assignment::value)
            {
                if (m_allocator == other.m_allocator)
                {
                    clear();
                }
                else
                {
                    clear_and_deallocate_memory();
                    m_allocator = other.m_allocator;
                }
            }
            else
            {
                clear();
            }

            m_hash = other.m_hash;
            m_key_equal = other.m_key_equal;

            reserve(other.m_count);

            for (std::pair<Key_t const, Value_t> const& keyValue : other)
            {
                insert_or_assign(keyValue.first, keyValue.second);
            }

            return *this;
        }

        /**
         * @brief Move assignment.
         * 
         * Destroy all keys and values and deallocate all memory. Move hash and
         * key comparison functions.
         * If std::allocator_traits<Allocator_t>::propagate_on_container_move_assignment
         * is true, then reuse memory and content of @other and move its
         * allocator. If it's false and the allocators are equal, then also
         * reuse memory and content of @other. Otherwise allocate the necessary
         * memory and move the elements of @other into the new memory.
         * 
         * @param other is the hash map to move.
         * @return Hash_map& is a reference to itself.
         */
        Hash_map& operator=(Hash_map&& other) noexcept
        {
            // TODO test custom key and hash map are moved
            // TODO test possible scenarios
            // TODO test all objects are destructed

            clear_and_deallocate_memory();

            m_hash = std::move(other.m_hash);
            m_key_equal = std::move(other.m_key_equal);

            using Propagate_on_container_move_assignment =
                typename std::allocator_traits<Allocator_t>::propagate_on_container_move_assignment;

            if constexpr (Propagate_on_container_move_assignment::value)
            {
                m_content = std::exchange(other.m_content, nullptr);
                m_is_content_valid = std::exchange(other.m_is_content_valid, nullptr);
                m_capacity = std::exchange(other.m_capacity, 0);
                m_count = std::exchange(other.m_count, 0);
                m_allocator = std::move(other.m_allocator);
            }
            else
            {
                if (m_allocator == other.m_allocator)
                {
                    m_content = std::exchange(other.m_content, nullptr);
                    m_is_content_valid = std::exchange(other.m_is_content_valid, nullptr);
                    m_capacity = std::exchange(other.m_capacity, 0);
                    m_count = std::exchange(other.m_count, 0);
                }
                else
                {
                    reserve(other.m_count);

                    for (auto iterator = other.begin(); iterator != other.end(); ++iterator)
                    {
                        insert_or_assign(std::move(iterator->first), std::move(iterator->second));
                    }
                }
            }

            return *this;
        }

        /**
         * @brief Return the number of elements that the container has currently allocated space for.
         * 
         * @return std::size_t Capacity of the currently allocated storage.
         */
        std::size_t capacity() const noexcept
        {
            return calculate_allocated_count(m_capacity);
        }

        /**
         * @brief Check if the container contains no elements.
         * 
         * @return true if the container is empty
         * @return false otherwise
         */
        bool empty() const noexcept
        {
            return size() == 0;
        }

        /**
         * @brief Return the number of key-value pairs in the container.
         * 
         * @return std::size_t The number of key-value pairs in the container.
         */
        std::size_t size() const noexcept
        {
            return m_count;
        }

        /**
         * @brief Add a key-value pair to the hash map.
         * 
         * If an equivalent key already exists, then the value is replaced.
         * 
         * @param key is the key used both to look up and to insert if not found.
         * @param value is a value to insert or assign.
         * @return std::pair<iterator, bool> The bool component is true if
         * adding the element did not replace an existing one. The iterator
         * component is pointing at the element that was inserted or updated.
         */
        std::pair<iterator, bool> insert_or_assign(Key_t key, Value_t value)
        {
            if (should_resize(m_count + 1, m_capacity))
            {
                reserve(2 * m_count);
            }
            
            std::size_t index = calculate_index_hint(key);

            while (is_content_valid(index) && !is_same_key(index, key))
            {
                index = (index + 1) % m_capacity;
            }

            {
                std::pair<Key_t const, Value_t>* location = &m_content[index];
                std::construct_at(location, std::make_pair(std::move(key), std::move(value)));
            }

            bool const inserted = !is_content_valid(index);
            set_content_valid(index, true);

            m_count++;

            return
            {
                iterator{m_content, m_is_content_valid, m_capacity, index},
                inserted
            };
        }

        /**
         * @brief Return an iterator to the first element.
         * 
         * If the iterator is empty, the returned iterator will be equal to end().
         * 
         * @return iterator to the first element.
         */
        iterator begin() noexcept
        {
            std::size_t const first_valid_index = get_first_valid_index();

            return iterator{m_content, m_is_content_valid, m_capacity, first_valid_index};
        }

        /**
         * @brief Return an iterator to the first element.
         * 
         * If the iterator is empty, the returned iterator will be equal to end().
         * 
         * @return iterator to the first element.
         */
        const_iterator begin() const noexcept
        {
            std::size_t const first_valid_index = get_first_valid_index();

            return const_iterator{m_content, m_is_content_valid, m_capacity, first_valid_index};
        }

        /**
         * @brief Return an iterator to the element following the last element.
         * 
         * This iterator can be used to compare with other iterators. It cannot
         * be used to access the element it points to as it is invalid.
         * 
         * @return iterator to the element following the last element.
         */
        iterator end() noexcept
        {
            return iterator{m_content, m_is_content_valid, m_capacity, m_capacity};
        }

        /**
         * @brief Return an iterator to the element following the last element.
         * 
         * This iterator can be used to compare with other iterators. It cannot
         * be used to access the element it points to as it is invalid.
         * 
         * @return iterator to the element following the last element.
         */
        const_iterator end() const noexcept
        {
            return const_iterator{m_content, m_is_content_valid, m_capacity, m_capacity};
        }

        /**
         * @brief Find an element with key equivalent to @key.
         * 
         * @param key of the value to search for.
         * @return iterator to an element with key equivalent to @key. If it is
         * not found, the returned iterator is end().
         */
        iterator find(Key_t const& key) noexcept
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
                return iterator{m_content, m_is_content_valid, m_capacity, index};
            }
            else
            {
                return end();
            }
        }

        /**
         * @brief Find an element with key equivalent to @key.
         * 
         * @param key of the value to search for.
         * @return iterator to an element with key equivalent to @key. If it is
         * not found, the returned iterator is end().
         */
        const_iterator find(Key_t const& key) const noexcept
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
                return const_iterator{m_content, m_is_content_valid, m_capacity, index};
            }
            else
            {
                return end();
            }
        }

        /**
         * @brief Return a reference to the mapped value of the element with
         * key equivalent to @key.
         * 
         * If an element is not found, then an exception of type
         * std::out_of_range is thrown.
         * 
         * @param key of the value to search for.
         * @return Value_t& Reference to the mapped value that corresponds to
         * @key.
         */
        Value_t& at(Key_t const& key)
        {
            iterator const iterator = find(key);

            if (iterator != end())
            {
                return iterator->second;
            }
            else
            {
                throw std::out_of_range{"Key was not found!"};
            }
        }

        /**
         * @brief Return a reference to the mapped value of the element with
         * key equivalent to @key.
         * 
         * If an element is not found, then an exception of type
         * std::out_of_range is thrown.
         * 
         * @param key of the value to search for.
         * @return Value_t& Reference to the mapped value that corresponds to
         * @key.
         */
        Value_t const& at(Key_t const& key) const
        {
            const_iterator const iterator = find(key);

            if (iterator != end())
            {
                return iterator->second;
            }
            else
            {
                throw std::out_of_range{"Key was not found!"};
            }
        }

        /**
         * @brief Check if there is an element with key equivalent to @key.
         * 
         * @param key of the value to search for.
         * @return true if there is an element with key equivalent to @key.
         * @return false otherwise.
         */
        bool contains(Key_t const& key) const noexcept
        {
            const_iterator const iterator = find(key);

            return iterator != end();
        }

        /**
         * @brief Remove the element with key equivalent to @key if such an
         * element exists.
         * 
         * @param key of the value to search for.
         */
        void erase(Key_t const& key) noexcept
        {
            iterator const iterator = find(key);

            if (iterator != end())
            {
                {
                    std::size_t const index = iterator.m_index;
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

        /**
         * @brief Remove all elements from the container.
         * 
         */
        void clear() noexcept
        {
            for (auto iterator = begin(); iterator != end(); ++iterator)
            {
                std::destroy_at(&(*iterator));
            }

            m_count = 0;
            std::memset(m_is_content_valid, 0, m_capacity / 8);
        }

        /**
         * @brief Swap contents with @other.
         * 
         * If std::allocator_traits<allocator_type>::propagate_on_container_swap
         * is true, then the allocators are also swapped. Otherwise, they are
         * not.
         * 
         * @param other is the object to swap with.
         */
        void swap(Hash_map& other) noexcept
        {
            std::swap(m_content, other.m_content);
            std::swap(m_is_content_valid, other.m_is_content_valid);
            std::swap(m_capacity, other.m_capacity);
            std::swap(m_count, other.m_count);

            using Propagate_on_container_swap =
                typename std::allocator_traits<Allocator_t>::propagate_on_container_swap;

            if constexpr (Propagate_on_container_swap::value)
            {
                std::swap(m_allocator, other.m_allocator);
            }
        }

        /**
         * @brief Reserve memory for at least the specified number of elements.
         * 
         * @param count is the new capacity of the container.
         */
        void reserve(std::size_t const count)
        {
            std::size_t const old_capacity = m_capacity;
            std::size_t const new_capacity = calculate_capacity(count);

            if (old_capacity >= new_capacity)
            {
                return;
            }

            auto const old_content = detail::create_unique_ptr(m_content, m_allocator, old_capacity);
            
            using Byte_allocator = std::allocator_traits<decltype(m_allocator)>::template rebind_alloc<std::byte>;
            Byte_allocator bytes_allocator{m_allocator};
            
            constexpr std::size_t number_of_bits_in_a_byte = 8;
            std::size_t const old_content_is_valid_capacity = (old_capacity / number_of_bits_in_a_byte) + ((old_capacity % number_of_bits_in_a_byte) != 0 ? 1 : 0);
            auto const old_is_content_valid = detail::create_unique_ptr(m_is_content_valid, bytes_allocator, old_content_is_valid_capacity);

            {
                auto new_content = detail::create_unique_ptr(m_allocator, new_capacity);

                auto new_is_content_valid = [&]
                {
                    std::size_t const number_of_bytes_to_allocate = (new_capacity / number_of_bits_in_a_byte) + ((new_capacity % number_of_bits_in_a_byte) != 0 ? 1 : 0);
                    auto new_is_content_valid = detail::create_unique_ptr(bytes_allocator, number_of_bytes_to_allocate);

                    std::memset(new_is_content_valid.get(), 0, number_of_bytes_to_allocate);
                    
                    return new_is_content_valid;
                }();

                m_content = new_content.release();
                m_is_content_valid = new_is_content_valid.release();
                m_capacity = new_capacity;
            }

            if (old_content.get() != nullptr)
            {
                m_count = 0;

                {
                    iterator const old_content_begin{old_content.get(), old_is_content_valid.get(), old_capacity, get_first_valid_index(old_is_content_valid.get(), old_capacity)};
                    iterator const old_content_end{old_content.get(), old_is_content_valid.get(), old_capacity, old_capacity};
                    
                    for (auto iterator = old_content_begin; iterator != old_content_end; ++iterator)
                    {
                        insert_or_assign(std::move(iterator->first), std::move(iterator->second));
                        std::destroy_at(&(*iterator));
                    }
                }
            }
        }

        /**
         * @brief Return the associated allocator.
         * 
         * @return Allocator_t const& The associated allocator.
         */
        Allocator_t const& get_allocator() const noexcept
        {
            return m_allocator;
        }

        /**
         * @brief Get the required memory that is needed to reserve
         * memory for the specified number of elements.
         * 
         * @param count is the capacity of the container.
         * @return std::size_t The maximum number of bytes that will be
         * allocated when calling reserve() given @count.
         */
        static constexpr std::size_t get_required_memory_size(std::size_t const count) noexcept
        {
            std::size_t const required_capacity = calculate_capacity(count);

            std::size_t const alignment_offset = alignof(std::pair<Key_t const, Value_t>) - 1;

            return required_capacity * sizeof(std::pair<Key_t const, Value_t>) + required_capacity / 8 + ((required_capacity % 8 != 0) ? 1 : 0) + alignment_offset;
        }

    private:

        void clear_and_deallocate_memory() noexcept
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

            return m_key_equal(pair.first, key);
        }

        std::size_t calculate_index_hint(Key_t const& key) const noexcept
        {
            assert(m_capacity != 0);

            std::size_t const hash_value = m_hash(key);
            
            return hash_value % m_capacity;
        }

        static constexpr std::size_t calculate_capacity(std::size_t count) noexcept
        {
            constexpr std::size_t load_factor = 70;
            return std::max<std::size_t>(count, c_minimum_number_of_elements) * 100 / load_factor;
        }

        static constexpr std::size_t calculate_allocated_count(std::size_t capacity) noexcept
        {
            constexpr std::size_t load_factor = 70;
            std::size_t const numerator = capacity * load_factor;
            std::size_t const denominator = 100;

            return numerator / denominator + ((numerator % denominator) != 0 ? 1 : 0);
        }

    private:

        std::pair<Key_t const, Value_t>* m_content = nullptr;
        std::byte* m_is_content_valid = nullptr;
        std::size_t m_capacity = 0;
        std::size_t m_count = 0;
        Allocator_t m_allocator;
        Hash_t m_hash;
        Key_equal_t m_key_equal;

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
