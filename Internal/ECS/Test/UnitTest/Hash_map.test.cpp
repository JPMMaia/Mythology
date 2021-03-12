#include <catch2/catch.hpp>

#include <array>
#include <cstddef>
#include <memory_resource>
#include <utility>

import maia.ecs.hash_map;

namespace Maia::ECS::Test
{
    TEST_CASE("Hash_map.empty checks if it does not contain any element", "[hash_map]")
    {
        Hash_map<int, int> hash_map;

        CHECK(hash_map.empty());

        hash_map.insert_or_assign({1, 2});

        CHECK(!hash_map.empty());
    }

    TEST_CASE("Hash_map.size returns the number of elements", "[hash_map]")
    {
        Hash_map<int, int> hash_map;

        CHECK(hash_map.size() == 0);

        hash_map.insert_or_assign({1, 2});

        CHECK(hash_map.size() == 1);
    }

    TEST_CASE("Hash_map.insert_or_assign {adds a new key-value pair", "[hash_map]}")
    {
        Hash_map<int, int> hash_map;

        hash_map.insert_or_assign({1, 2});
        CHECK(hash_map.size() == 1);
    }

    TEST_CASE("Hash_map.clear removes all elements", "[hash_map]")
    {
        Hash_map<int, int> hash_map;

        hash_map.insert_or_assign({1, 2});
        hash_map.insert_or_assign({2, 2});
        CHECK(hash_map.size() == 2);

        hash_map.clear();
        CHECK(hash_map.empty());

        CHECK(!hash_map.contains(1));
        CHECK(!hash_map.contains(2));
    }

    TEST_CASE("Hash_map.begin and Hash_map.end are the same if empty", "[hash_map]")
    {
        Hash_map<int, int> hash_map;
        
        CHECK(hash_map.begin() == hash_map.end());
    }

    TEST_CASE("Hash_map.begin and Hash_map.end return iterators to existing elements", "[hash_map]")
    {
        Hash_map<int, int> hash_map;

        hash_map.insert_or_assign({1, 2});
        hash_map.insert_or_assign({2, 1});

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

        hash_map.insert_or_assign({1, 2});
        hash_map.insert_or_assign({2, 1});

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

        hash_map.insert_or_assign({1, 2});
        hash_map.insert_or_assign({2, 1});

        CHECK(hash_map.at(1) == 2);
        CHECK(hash_map.at(2) == 1);

        Hash_map<int, int> const& const_hash_map = hash_map;

        CHECK(const_hash_map.at(1) == 2);
        CHECK(const_hash_map.at(2) == 1);
    }

    TEST_CASE("Hash_map.contains checks if the container contains an element", "[hash_map]")
    {
        Hash_map<int, int> hash_map;

        hash_map.insert_or_assign({1, 2});
        hash_map.insert_or_assign({2, 1});

        CHECK(hash_map.contains(1));
        CHECK(hash_map.contains(2));
        CHECK(!hash_map.contains(3));
    }

    TEST_CASE("Hash_map.erase removes an element", "[hash_map]")
    {
        Hash_map<int, int> hash_map;

        hash_map.insert_or_assign({1, 2});
        hash_map.insert_or_assign({2, 1});

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
            hash_map.insert_or_assign({static_cast<int>(index), static_cast<int>(index)});
        }

        for (std::size_t index = 0; index < element_count; ++index)
        {
            hash_map.erase(index);
        }

        CHECK(hash_map.size() == 0);
    }

    TEST_CASE("Hash_map.swap swaps the content with another hash map", "[hash_map]")
    {
        Hash_map<int, int> hash_map_a;
        hash_map_a.insert_or_assign({1, 3});
        hash_map_a.insert_or_assign({2, 2});

        Hash_map<int, int> hash_map_b;
        hash_map_b.insert_or_assign({3, 1});
        hash_map_b.insert_or_assign({4, 0});

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

    TEST_CASE("Hash_map.reserve reserves memory to accomodate elements", "[hash_map]")
    {
        constexpr std::size_t number_of_elements = 48;
        constexpr std::size_t required_memory_size = Hash_map<int, int>::get_required_memory_size(number_of_elements);

        std::array<std::byte, required_memory_size> buffer_storage;
        std::pmr::monotonic_buffer_resource buffer_resource{buffer_storage.data(), buffer_storage.size(), std::pmr::null_memory_resource()};
        std::pmr::polymorphic_allocator<> allocator{&buffer_resource};

        auto const add_elements = [number_of_elements](pmr::Hash_map<int, int>& hash_map) -> void
        {
            for (std::size_t element_index = 0; element_index < number_of_elements; ++element_index)
            {
                hash_map.insert_or_assign({static_cast<int>(element_index), static_cast<int>(element_index)});
            }
        };

        auto const remove_elements = [number_of_elements](pmr::Hash_map<int, int>& hash_map) -> void
        {
            for (std::size_t element_index = 0; element_index < number_of_elements; ++element_index)
            {
                hash_map.erase(element_index);
            }
        };

        auto const add_and_clear_multiple_times = [&]() -> void
        {
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

    TEST_CASE("Calling Hash_map.reserve twice does not invalidate content", "[hash_map]")
    {
        constexpr std::size_t element_count = 100;

        Hash_map<std::size_t, std::size_t> hash_map;
        hash_map.reserve(element_count);

        for (std::size_t index = 0; index < element_count; ++index)
        {
            hash_map.insert_or_assign({index, index});
        }

        hash_map.reserve(element_count + element_count);

        for (std::size_t index = 0; index < element_count; ++index)
        {
            CHECK(hash_map.at(index) == index);
        }
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
            other.insert_or_assign({1, Copiable{2}});
            other.insert_or_assign({2, Copiable{1}});

            std::size_t const number_of_copies = copy_constructor_called;
            
            Hash_map<int, Copiable> copy{other};
            
            CHECK(copy_constructor_called == (number_of_copies + 2));
            
            return copy;
        }();

        CHECK(hash_map.at(1) == Copiable{2});
        CHECK(hash_map.at(2) == Copiable{1});
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
            other.insert_or_assign({1, Copiable{2}});
            other.insert_or_assign({2, Copiable{1}});

            Hash_map<int, Copiable> copy;
            copy.insert_or_assign({1, Copiable{3}});

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
            other.insert_or_assign({1, 2});
            other.insert_or_assign({2, 1});

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
            other.insert_or_assign({1, 2});
            other.insert_or_assign({2, 1});

            Hash_map<int, int> moved;
            moved.insert_or_assign({1, 3});
            moved = std::move(other);
            return moved;
        }();

        CHECK(hash_map.at(1) == 2);
        CHECK(hash_map.at(2) == 1);
    }
}
