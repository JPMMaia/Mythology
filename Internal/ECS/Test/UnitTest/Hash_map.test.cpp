#include <catch2/catch.hpp>

#include <array>
#include <cstddef>
#include <iterator>
#include <memory_resource>
#include <numeric>
#include <ranges>
#include <unordered_map>
#include <utility>

import maia.ecs.hash_map;
import maia.test.debug_object;
import maia.test.debug_resource;

namespace Maia::ECS::Test
{
    using Maia::Test::Debug_resource;
    using Maia::Test::Debug_object;

    TEST_CASE("Hash_map.capacity returns the number of reserved elements", "[hash_map]")
    {
        auto const test_capacity = [](std::size_t const capacity) -> void
        {
            Hash_map<int, int> hash_map;
            CHECK(hash_map.capacity() == 0);

            hash_map.reserve(capacity);
            CHECK(hash_map.capacity() == capacity);
            hash_map.insert_or_assign(1, 2);
            CHECK(hash_map.capacity() == capacity);
        };

        test_capacity(16);
        test_capacity(17);
        test_capacity(18);
        test_capacity(19);
        test_capacity(70);
        test_capacity(100);
        test_capacity(101);
    }

    TEST_CASE("Hash_map.empty checks if it does not contain any element", "[hash_map]")
    {
        Hash_map<int, int> hash_map;

        CHECK(hash_map.empty());

        hash_map.insert_or_assign(1, 2);

        CHECK(!hash_map.empty());
    }

    TEST_CASE("Hash_map.size returns the number of elements", "[hash_map]")
    {
        Hash_map<int, int> hash_map;

        CHECK(hash_map.size() == 0);

        hash_map.insert_or_assign(1, 2);

        CHECK(hash_map.size() == 1);
    }

    TEST_CASE("Hash_map.insert_or_assign dds a new key-value pair", "[hash_map]")
    {
        Hash_map<int, int> hash_map;

        hash_map.insert_or_assign(1, 2);
        CHECK(hash_map.size() == 1);
    }

    TEST_CASE("Hash_map.insert_or_assign eturns iterator and whether it inserted", "[hash_map]")
    {
        Hash_map<int, int> hash_map;

        {
            auto const result = hash_map.insert_or_assign(1, 2);
        
            auto const iterator = result.first;
            CHECK(iterator->first == 1);
            CHECK(iterator->second == 2);

            CHECK(result.second);
        }

        {
            auto const result = hash_map.insert_or_assign(1, 3);
        
            auto const iterator = result.first;
            CHECK(iterator->first == 1);
            CHECK(iterator->second == 3);

            CHECK(!result.second);
        }
    }

    TEST_CASE("Hash_map.clear removes all elements", "[hash_map]")
    {
        Hash_map<int, int> hash_map;

        hash_map.insert_or_assign(1, 2);
        hash_map.insert_or_assign(2, 2);
        CHECK(hash_map.size() == 2);

        hash_map.clear();
        CHECK(hash_map.empty());

        CHECK(!hash_map.contains(1));
        CHECK(!hash_map.contains(2));
    }

    TEST_CASE("Hash_map.clear destroys all objects", "[hash_map]")
    {
        std::size_t constructor_counter = 0;
        std::size_t destructor_counter = 0;

        pmr::Hash_map<int, Debug_object> hash_map;
        hash_map.reserve(16);
        hash_map.insert_or_assign(1, Debug_object{&constructor_counter, &destructor_counter});
        hash_map.insert_or_assign(2, Debug_object{&constructor_counter, &destructor_counter});
        hash_map.clear();

        CHECK(constructor_counter != 0);
        CHECK(constructor_counter == destructor_counter);
    }


    TEST_CASE("Hash_map.begin and Hash_map.end are the same if empty", "[hash_map]")
    {
        Hash_map<int, int> hash_map;
        
        CHECK(hash_map.begin() == hash_map.end());
    }

    TEST_CASE("Hash_map.begin and Hash_map.end return iterators to existing elements", "[hash_map]")
    {
        Hash_map<int, int> hash_map;

        hash_map.insert_or_assign(1, 2);
        hash_map.insert_or_assign(2, 1);

        CHECK(hash_map.begin() != hash_map.end());
        CHECK(std::distance(hash_map.begin(), hash_map.end()) == 2);

        {
            auto const is_valid = [](auto& hash_map, auto const iterator) -> bool
            {
                if (iterator == hash_map.end())
                {
                    return false;
                }
                else if (iterator->first == 1)
                {
                    return iterator->second == 2;
                }
                else if (iterator->first == 2)
                {
                    return iterator->second == 1;
                }
                else
                {
                    return false;
                }
            };

            {
                auto iterator = hash_map.begin();
                REQUIRE(iterator != hash_map.end());
                CHECK(is_valid(hash_map, iterator));

                ++iterator;
                REQUIRE(iterator != hash_map.end());
                CHECK(is_valid(hash_map, iterator));

                ++iterator;
                CHECK(iterator == hash_map.end());
            }

            {
                Hash_map<int, int> const& const_hash_map = hash_map;

                auto iterator = const_hash_map.begin();
                REQUIRE(iterator != const_hash_map.end());
                CHECK(is_valid(const_hash_map, iterator));

                ++iterator;
                REQUIRE(iterator != const_hash_map.end());
                CHECK(is_valid(const_hash_map, iterator));

                ++iterator;
                CHECK(iterator == const_hash_map.end());
            }
        }
    }

    TEST_CASE("Hash_map.find returns an iterator to an element", "[hash_map]")
    {
        Hash_map<int, int> hash_map;

        CHECK(hash_map.find(1) == hash_map.end());
        CHECK(std::as_const(hash_map).find(1) == std::as_const(hash_map).end());

        hash_map.insert_or_assign(1, 2);
        hash_map.insert_or_assign(2, 1);

        {
            auto const iterator = hash_map.find(1);
            
            REQUIRE(iterator != hash_map.end());
            CHECK(iterator->first == 1);
            CHECK(iterator->second == 2);
        }

        {
            auto const iterator = hash_map.find(2);
            
            REQUIRE(iterator != hash_map.end());
            CHECK(iterator->first == 2);
            CHECK(iterator->second == 1);
        }

        {
            auto const iterator = hash_map.find(3);
            
            CHECK(iterator == hash_map.end());
        }

        Hash_map<int, int> const& const_hash_map = hash_map;

        {
            auto const iterator = const_hash_map.find(1);
            
            REQUIRE(iterator != const_hash_map.end());
            CHECK(iterator->first == 1);
            CHECK(iterator->second == 2);
        }

        {
            auto const iterator = const_hash_map.find(2);
            
            REQUIRE(iterator != const_hash_map.end());
            CHECK(iterator->first == 2);
            CHECK(iterator->second == 1);
        }

        {
            auto const iterator = const_hash_map.find(3);
            
            CHECK(iterator == const_hash_map.end());
        }
    }

    TEST_CASE("Hash_map.at can be used to access an element", "[hash_map]")
    {
        Hash_map<int, int> hash_map;

        hash_map.insert_or_assign(1, 2);
        hash_map.insert_or_assign(2, 1);

        CHECK(hash_map.at(1) == 2);
        CHECK(hash_map.at(2) == 1);

        Hash_map<int, int> const& const_hash_map = hash_map;

        CHECK(const_hash_map.at(1) == 2);
        CHECK(const_hash_map.at(2) == 1);
    }

    TEST_CASE("Hash_map.at throws exception if key does not exist", "[hash_map]")
    {
        Hash_map<int, int> hash_map;
        CHECK_THROWS_AS(hash_map.at(1), std::out_of_range);

        Hash_map<int, int> const& const_hash_map = hash_map;
        CHECK_THROWS_AS(const_hash_map.at(1), std::out_of_range);
    }

    TEST_CASE("Hash_map.contains checks if the container contains an element", "[hash_map]")
    {
        Hash_map<int, int> hash_map;

        hash_map.insert_or_assign(1, 2);
        hash_map.insert_or_assign(2, 1);

        CHECK(hash_map.contains(1));
        CHECK(hash_map.contains(2));
        CHECK(!hash_map.contains(3));
    }

    TEST_CASE("Hash_map.erase removes an element", "[hash_map]")
    {
        Hash_map<int, int> hash_map;

        hash_map.insert_or_assign(1, 2);
        hash_map.insert_or_assign(2, 1);

        REQUIRE(hash_map.contains(1));
        REQUIRE(hash_map.contains(2));

        hash_map.erase(1);
        
        CHECK(!hash_map.contains(1));
        CHECK(hash_map.at(2) == 1);
    }

    TEST_CASE("Hash_map.erase does not invalidate other elemenets", "[hash_map]")
    {
        constexpr std::size_t element_count = 100;

        Hash_map<int, int> hash_map;
        hash_map.reserve(element_count);

        for (std::size_t index = 0; index < element_count; ++index)
        {
            hash_map.insert_or_assign(static_cast<int>(index), static_cast<int>(index));
        }

        for (std::size_t index = 0; index < element_count; ++index)
        {
            hash_map.erase(index);
        }

        CHECK(hash_map.size() == 0);
    }

    TEST_CASE("Hash_map.erase calls destructor", "[hash_map]")
    {
        std::size_t constructor_counter = 0;
        std::size_t destructor_counter = 0;

        auto const create_debug_object = [&]() -> Debug_object
        {
            return {&constructor_counter, &destructor_counter};
        };

        Hash_map<int, Debug_object> hash_map;
        hash_map.reserve(2);
        hash_map.insert_or_assign(1, create_debug_object());
        hash_map.insert_or_assign(2, create_debug_object());

        CHECK((constructor_counter - destructor_counter) == 2);
        hash_map.erase(1);
        CHECK((constructor_counter - destructor_counter) == 1);
        hash_map.erase(2);
        CHECK((constructor_counter - destructor_counter) == 0);
    }

    TEST_CASE("Hash_map.swap swaps the content with another hash map", "[hash_map]")
    {
        Hash_map<int, int> hash_map_a;
        hash_map_a.insert_or_assign(1, 3);
        hash_map_a.insert_or_assign(2, 2);

        Hash_map<int, int> hash_map_b;
        hash_map_b.insert_or_assign(3, 1);
        hash_map_b.insert_or_assign(4, 0);

        hash_map_a.swap(hash_map_b);

        CHECK(!hash_map_a.contains(1));
        CHECK(!hash_map_a.contains(2));
        CHECK(hash_map_a.at(3) == 1);
        CHECK(hash_map_a.at(4) == 0);

        CHECK(hash_map_b.at(1) == 3);
        CHECK(hash_map_b.at(2) == 2);
        CHECK(!hash_map_b.contains(3));
        CHECK(!hash_map_b.contains(4));
    }

    namespace
    {
        template <std::size_t number_of_elements>
        void test_reserve()
        {
            auto const add_elements = [](pmr::Hash_map<int, int>& hash_map) -> void
            {
                for (std::size_t element_index = 0; element_index < number_of_elements; ++element_index)
                {
                    hash_map.insert_or_assign(static_cast<int>(element_index), static_cast<int>(element_index));
                }
            };

            auto const remove_elements = [](pmr::Hash_map<int, int>& hash_map) -> void
            {
                for (std::size_t element_index = 0; element_index < number_of_elements; ++element_index)
                {
                    hash_map.erase(element_index);
                }
            };

            auto const add_and_clear_multiple_times = [&]() -> void
            {
                constexpr std::size_t required_memory_size = Hash_map<int, int>::get_required_memory_size(number_of_elements);

                std::array<std::byte, required_memory_size> buffer_storage;
                std::pmr::monotonic_buffer_resource buffer_resource{buffer_storage.data(), buffer_storage.size(), std::pmr::null_memory_resource()};
                std::pmr::polymorphic_allocator<> allocator{&buffer_resource};

                pmr::Hash_map<int, int> hash_map{allocator};
                hash_map.reserve(number_of_elements);

                for (std::size_t i = 0; i < 100; ++i)
                {
                    add_elements(hash_map);
                    remove_elements(hash_map);
                }

                for (std::size_t i = 0; i < 100; ++i)
                {
                    add_elements(hash_map);
                    hash_map.clear();
                }
            };

            CHECK_NOTHROW(add_and_clear_multiple_times());
        }
    }
    
    TEST_CASE("Hash_map.reserve reserves memory to accomodate elements", "[hash_map]")
    {
        test_reserve<2>();
        test_reserve<16>();
        test_reserve<48>();
    }

    TEST_CASE("Hash_map.reserve calls constructors and destructors", "[hash_map]")
    {
        std::size_t constructor_counter = 0;
        std::size_t destructor_counter = 0;

        auto const create_debug_object = [&]() -> Debug_object
        {
            return {&constructor_counter, &destructor_counter};
        };

        {
            Hash_map<int, Debug_object> hash_map;
            hash_map.reserve(16);
            hash_map.insert_or_assign(1, create_debug_object());
            hash_map.insert_or_assign(2, create_debug_object());

            CHECK((constructor_counter - destructor_counter) == 2);

            hash_map.reserve(32);

            CHECK((constructor_counter - destructor_counter) == 2);
        }

        CHECK(constructor_counter > 0);
        CHECK(constructor_counter == destructor_counter);
    }

    TEST_CASE("Hash_map.reserve releases memory", "[hash_map]")
    {
        std::size_t allocated_bytes_counter = 0;
        std::size_t deallocated_bytes_counter = 0;
        Debug_resource debug_resource{&allocated_bytes_counter, &deallocated_bytes_counter};
        std::pmr::polymorphic_allocator<> allocator{&debug_resource};

        {
            pmr::Hash_map<int, int> hash_map{allocator};
            hash_map.reserve(16);
            hash_map.reserve(32);
            hash_map.reserve(64);
        }

        CHECK(allocated_bytes_counter != 0);
        CHECK(allocated_bytes_counter == deallocated_bytes_counter);
    }

    TEST_CASE("Calling Hash_map.reserve twice does not invalidate content", "[hash_map]")
    {
        constexpr std::size_t element_count = 100;

        Hash_map<std::size_t, std::size_t> hash_map;
        hash_map.reserve(element_count);

        for (std::size_t index = 0; index < element_count; ++index)
        {
            hash_map.insert_or_assign(index, index);
        }

        hash_map.reserve(element_count + element_count);

        for (std::size_t index = 0; index < element_count; ++index)
        {
            CHECK(hash_map.at(index) == index);
        }
    }

    TEST_CASE("Hash_map.reserve does not leak memory if it fails", "[hash_map]")
    {
        constexpr std::size_t number_of_elements = 16;
        constexpr std::size_t required_memory_size = Hash_map<int, int>::get_required_memory_size(number_of_elements);

        std::size_t allocated_bytes_counter = 0;
        std::size_t deallocated_bytes_counter = 0;
        std::size_t remaining_memory = required_memory_size - 4;
        Debug_resource debug_resource{&allocated_bytes_counter, &deallocated_bytes_counter, &remaining_memory};
        std::pmr::polymorphic_allocator<> allocator{&debug_resource};

        {
            pmr::Hash_map<int, int> hash_map{allocator};
            REQUIRE_THROWS_AS(hash_map.reserve(number_of_elements), std::bad_alloc);
        }

        CHECK(allocated_bytes_counter == deallocated_bytes_counter);
    }

    TEST_CASE("Hash_map.get_required_memory_size takes alignment into account", "[hash_map]")
    {
        constexpr std::array<std::size_t, 3> number_of_elements
        {
            20,
            21,
            22
        };

        constexpr std::array<std::size_t, 3> required_memory_sizes
        {
           Hash_map<int, int>::get_required_memory_size(number_of_elements[0]),
           Hash_map<int, int>::get_required_memory_size(number_of_elements[1]),
           Hash_map<int, int>::get_required_memory_size(number_of_elements[2]),
        };

        constexpr std::size_t total_required_memory_size = std::accumulate(required_memory_sizes.begin(), required_memory_sizes.end(), std::size_t{0}) + 1;

        std::array<std::byte, total_required_memory_size> buffer_storage;
        std::pmr::monotonic_buffer_resource buffer_resource{buffer_storage.data(), buffer_storage.size(), std::pmr::null_memory_resource()};
        std::pmr::polymorphic_allocator<> allocator{&buffer_resource};
        
        {
            void* const pointer = allocator.allocate_bytes(1, 1);
            allocator.deallocate_bytes(pointer, 1, 1);
        }

        pmr::Hash_map<int, int> hash_map{allocator};
        CHECK_NOTHROW(hash_map.reserve(number_of_elements[0]));
        CHECK_NOTHROW(hash_map.reserve(number_of_elements[1]));
        CHECK_NOTHROW(hash_map.reserve(number_of_elements[2]));
    }

    TEST_CASE("Hash_map copy constructor copies content", "[hash_map]")
    {
        static std::size_t copy_constructor_called = 0;

        struct Copiable
        {
            Copiable() noexcept = default;

            explicit Copiable(int const value) noexcept :
                value{value}
            {
            }

            Copiable(Copiable const& other) noexcept :
                value{other.value}
            {
                ++copy_constructor_called;
            }

            Copiable(Copiable&& other) noexcept = default;
            Copiable& operator=(Copiable const& other) noexcept = default;
            Copiable& operator=(Copiable&& other) noexcept = default;

            bool operator==(Copiable const& rhs) const noexcept 
            {
                return value == rhs.value;
            }

            int value = 0;
        };

        Hash_map<int, Copiable> hash_map = []() -> Hash_map<int, Copiable>
        {
            Hash_map<int, Copiable> other;
            other.insert_or_assign(1, Copiable{2});
            other.insert_or_assign(2, Copiable{1});

            std::size_t const number_of_copies = copy_constructor_called;
            
            Hash_map<int, Copiable> copy{other};
            
            CHECK(copy_constructor_called == (number_of_copies + 2));
            
            return copy;
        }();

        CHECK(hash_map.at(1) == Copiable{2});
        CHECK(hash_map.at(2) == Copiable{1});
    }

    TEST_CASE("Hash_map copy constructor copies allocator", "[hash_map]")
    {
        std::size_t allocated_bytes_counter = 0;
        std::size_t deallocated_bytes_counter = 0;
        Debug_resource debug_resource{&allocated_bytes_counter, &deallocated_bytes_counter};
        std::pmr::polymorphic_allocator<> allocator{&debug_resource};
        
        pmr::Hash_map<int, int> other{allocator};
        
        pmr::Hash_map<int, int> copy{other};
        
        CHECK(copy.get_allocator() == allocator);
    }

    TEST_CASE("Hash_map copy assignment copies content", "[hash_map]")
    {
        static std::size_t copy_constructor_called = 0;

        struct Copiable
        {
            Copiable() noexcept = default;

            explicit Copiable(int const value) noexcept :
                value{value}
            {
            }

            Copiable(Copiable const& other) noexcept :
                value{other.value}
            {
                ++copy_constructor_called;
            }

            Copiable(Copiable&& other) noexcept = default;
            Copiable& operator=(Copiable const& other) noexcept = default;
            Copiable& operator=(Copiable&& other) noexcept = default;

            bool operator==(Copiable const& rhs) const noexcept 
            {
                return value == rhs.value;
            }

            int value = 0;
        };

        Hash_map<int, Copiable> hash_map = []() -> Hash_map<int, Copiable>
        {
            Hash_map<int, Copiable> other;
            other.insert_or_assign(1, Copiable{2});
            other.insert_or_assign(2, Copiable{1});

            Hash_map<int, Copiable> copy;
            copy.insert_or_assign(1, Copiable{3});

            std::size_t const number_of_copies = copy_constructor_called;
            copy = other;
            CHECK(copy_constructor_called == (number_of_copies + 2));

            return copy;
        }();

        CHECK(hash_map.at(1) == Copiable{2});
        CHECK(hash_map.at(2) == Copiable{1});
    }

    TEST_CASE("Hash_map move constructor moves content", "[hash_map]")
    {
        Hash_map<int, int> hash_map = []() -> Hash_map<int, int>
        {
            Hash_map<int, int> other;
            other.insert_or_assign(1, 2);
            other.insert_or_assign(2, 1);

            Hash_map<int, int> moved{std::move(other)};
            return moved;
        }();

        CHECK(hash_map.at(1) == 2);
        CHECK(hash_map.at(2) == 1);
    }

    TEST_CASE("Hash_map move assignment moves content", "[hash_map]")
    {
        Hash_map<int, int> hash_map = []() -> Hash_map<int, int>
        {
            Hash_map<int, int> other;
            other.insert_or_assign(1, 2);
            other.insert_or_assign(2, 1);

            Hash_map<int, int> moved;
            moved.insert_or_assign(1, 3);
            moved = std::move(other);
            return moved;
        }();

        CHECK(hash_map.at(1) == 2);
        CHECK(hash_map.at(2) == 1);
    }

    TEST_CASE("Hash_map move assignment releases all memory", "[hash_map]")
    {
        std::size_t allocated_bytes_counter = 0;
        std::size_t deallocated_bytes_counter = 0;
        Debug_resource debug_resource{&allocated_bytes_counter, &deallocated_bytes_counter};
        std::pmr::polymorphic_allocator<> allocator{&debug_resource};

        {
            pmr::Hash_map<int, int> hash_map_0{allocator};
            hash_map_0.reserve(16);

            pmr::Hash_map<int, int> hash_map_1{allocator};
            hash_map_1.reserve(32);
            hash_map_1 = std::move(hash_map_0);
        }

        CHECK(allocated_bytes_counter != 0);
        CHECK(allocated_bytes_counter == deallocated_bytes_counter);
    }

    TEST_CASE("Hash_map destructor releases all memory", "[hash_map]")
    {
        std::size_t allocated_bytes_counter = 0;
        std::size_t deallocated_bytes_counter = 0;
        Debug_resource debug_resource{&allocated_bytes_counter, &deallocated_bytes_counter};
        std::pmr::polymorphic_allocator<> allocator{&debug_resource};

        {
            pmr::Hash_map<int, int> hash_map{allocator};
            hash_map.reserve(16);
        }

        CHECK(allocated_bytes_counter != 0);
        CHECK(allocated_bytes_counter == deallocated_bytes_counter);
    }

    TEST_CASE("Hash_map destructor destroys all objects", "[hash_map]")
    {
        std::size_t constructor_counter = 0;
        std::size_t destructor_counter = 0;

        {
            pmr::Hash_map<int, Debug_object> hash_map;
            hash_map.reserve(16);
            hash_map.insert_or_assign(1, Debug_object{&constructor_counter, &destructor_counter});
            hash_map.insert_or_assign(2, Debug_object{&constructor_counter, &destructor_counter});
        }

        CHECK(constructor_counter != 0);
        CHECK(constructor_counter == destructor_counter);
    }

    TEST_CASE("Hash_map uses custom hash and equal functions", "[hash_map]")
    {
        struct Custom_key
        {
            int value = 0;
        };

        bool hash_used = false;
        auto const hash = [&hash_used](Custom_key const value) -> std::size_t
        {
            hash_used = true;
            return std::hash<std::size_t>{}(value.value);
        };

        bool equal_used = false;
        auto const equal = [&equal_used](Custom_key const lhs, Custom_key const rhs) -> std::size_t
        {
            equal_used = true;
            return lhs.value == rhs.value;
        };

        Hash_map<Custom_key, int, decltype(hash), decltype(equal)> hash_map{hash, equal};
        hash_map.insert_or_assign(Custom_key{0}, 1);
        hash_map.insert_or_assign(Custom_key{1}, 0);
        hash_map.insert_or_assign(Custom_key{1}, 1);

        CHECK(hash_used);
        CHECK(equal_used);
    }

    TEST_CASE("Hash_map_iterator pre-increment", "[hash_map]")
    {
        Hash_map<int, int> hash_map;
        hash_map.insert_or_assign(1, 2);
        hash_map.insert_or_assign(2, 1);

        auto iterator = hash_map.begin();

        {
            auto const previous = iterator;
            CHECK(++iterator != previous);
            CHECK(iterator != previous);
        }

        CHECK(++iterator == hash_map.end());
        CHECK(iterator == hash_map.end());
    }

    TEST_CASE("Hash_map_iterator post-increment", "[hash_map]")
    {
        Hash_map<int, int> hash_map;
        hash_map.insert_or_assign(1, 2);
        hash_map.insert_or_assign(2, 1);

        auto iterator = hash_map.begin();

        {
            auto const previous = iterator;
            CHECK(iterator++ == previous);
            CHECK(iterator != previous);
        }

        {
            auto const previous = iterator;
            CHECK(iterator++ == previous);
            CHECK(iterator != previous);
            CHECK(iterator == hash_map.end());
        }
    }

    TEST_CASE("Hash_map_iterator pre-decrement", "[hash_map]")
    {
        Hash_map<int, int> hash_map;
        hash_map.insert_or_assign(1, 2);
        hash_map.insert_or_assign(2, 1);

        auto iterator = hash_map.begin();
        ++iterator;
        ++iterator;

        {
            auto const next = iterator;
            CHECK(--iterator != next);
            CHECK(iterator != next);
        }

        {
            auto const next = iterator;
            CHECK(--iterator != next);
            CHECK(iterator != next);
            auto begin = hash_map.begin();
            CHECK(iterator == hash_map.begin());
        }
    }

    TEST_CASE("Hash_map_iterator post-decrement", "[hash_map]")
    {
        Hash_map<int, int> hash_map;
        hash_map.insert_or_assign(1, 2);
        hash_map.insert_or_assign(2, 1);

        auto iterator = hash_map.begin();
        ++iterator;
        ++iterator;

        {
            auto const next = iterator;
            CHECK(iterator-- == next);
            CHECK(iterator != next);
        }

        {
            auto const next = iterator;
            CHECK(iterator-- == next);
            CHECK(iterator != next);
            CHECK(iterator == hash_map.begin());
        }
    }


    TEST_CASE("Hash_map.insert_or_assign benchmark", "[hash_map][benchmark]")
    {
        auto const benchmark = [](auto& hash_map, std::size_t const size)
        {
            hash_map.reserve(size);

            for (int i = 0; i < size; ++i)
            {
                hash_map.insert_or_assign(i, i);
            }
        };

        constexpr std::size_t size = 100000;

        BENCHMARK("Hash_map insert entries")
        {
            Hash_map<int, int> hash_map;
            return benchmark(hash_map, size);
        };

        BENCHMARK("Unordered_map insert entries")
        {
            std::unordered_map<int, int> hash_map;
            return benchmark(hash_map, size);
        };
    }

    TEST_CASE("Hash_map.at benchmark", "[hash_map][benchmark]")
    {
        auto const benchmark = [](Catch::Benchmark::Chronometer& meter, auto& hash_map, std::size_t const size)
        {
            hash_map.reserve(size);

            for (int i = 0; i < size; ++i)
            {
                hash_map.insert_or_assign(i, i);
            }

            meter.measure(
                [&hash_map, size]
                {
                    bool dummy = false;

                    for (int i = 0; i < size; ++i)
                    {
                        dummy = dummy && (hash_map.at(i) == i);
                    }

                    return dummy;
                }
            );

            return hash_map;
        };

        constexpr std::size_t size = 100000;

        BENCHMARK_ADVANCED("Hash_map find entries")(Catch::Benchmark::Chronometer meter)
        {
            Hash_map<int, int> hash_map;
            return benchmark(meter, hash_map, size);
        };

        BENCHMARK_ADVANCED("Unordered_map find entries")(Catch::Benchmark::Chronometer meter)
        {
            std::unordered_map<int, int> hash_map;
            return benchmark(meter, hash_map, size);
        };
    }

    TEST_CASE("Hash_map iteration benchmark", "[hash_map][benchmark]")
    {
        auto const benchmark = [](Catch::Benchmark::Chronometer& meter, auto& hash_map, std::size_t const size)
        {
            hash_map.reserve(size);

            for (int i = 0; i < size; ++i)
            {
                hash_map.insert_or_assign(i, i);
            }

            meter.measure(
                [&hash_map, size]
                {
                    int accummulator = 0;

                    for (auto iterator = hash_map.begin(); iterator != hash_map.end(); ++iterator)
                    {
                        accummulator += iterator->second;
                    }

                    return accummulator;
                }
            );

            return hash_map;
        };

        constexpr std::size_t size = 100000;

        BENCHMARK_ADVANCED("Hash_map iteration")(Catch::Benchmark::Chronometer meter)
        {
            Hash_map<int, int> hash_map;
            return benchmark(meter, hash_map, size);
        };

        BENCHMARK_ADVANCED("Unordered_map iteration")(Catch::Benchmark::Chronometer meter)
        {
            std::unordered_map<int, int> hash_map;
            return benchmark(meter, hash_map, size);
        };
    }

    TEST_CASE("Hash_map.erase benchmark", "[hash_map][benchmark]")
    {
        auto const benchmark = [](Catch::Benchmark::Chronometer& meter, auto& hash_map, std::size_t const size)
        {
            hash_map.reserve(size);

            for (int i = 0; i < size; ++i)
            {
                hash_map.insert_or_assign(i, i);
            }

            meter.measure(
                [&hash_map, size]
                {
                    for (int i = 0; i < size; ++i)
                    {
                        hash_map.erase(i);
                    }

                    return hash_map;
                }
            );

            return hash_map;
        };

        constexpr std::size_t size = 100000;

        BENCHMARK_ADVANCED("Hash_map erase")(Catch::Benchmark::Chronometer meter)
        {
            Hash_map<int, int> hash_map;
            return benchmark(meter, hash_map, size);
        };

        BENCHMARK_ADVANCED("Unordered_map erase")(Catch::Benchmark::Chronometer meter)
        {
            std::unordered_map<int, int> hash_map;
            return benchmark(meter, hash_map, size);
        };
    }
}
