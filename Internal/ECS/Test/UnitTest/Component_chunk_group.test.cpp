import maia.ecs.component;
import maia.ecs.component_chunk_group;
import maia.ecs.entity;

import <catch2/catch.hpp>;

import <array>;
import <cstring>;
import <iterator>;

namespace Maia::ECS::Test
{
    namespace
    {
        template <typename T>
        concept Has_value =
            requires (T t)
            {
                t.value;
            };

        template <Has_value T>
        std::ostream& operator<<(std::ostream& output_stream, T const component) noexcept
        {
            output_stream << component.value;
            
            return output_stream;
        }

        struct Component_a
        {
            int value = 1;

            auto operator<=>(Component_a const&) const noexcept = default;
        };

        struct Component_b
        {
            int value = 2;

            auto operator<=>(Component_b const&) const noexcept = default;
        };

        struct Component_c
        {
            int value = 3;

            auto operator<=>(Component_c const&) const noexcept = default;
        };

        struct Shared_component_d
        {
            int value = 4;

            auto operator<=>(Shared_component_d const&) const noexcept = default;
        };

        struct Shared_component_e
        {
            int value = 4;

            auto operator<=>(Shared_component_e const&) const noexcept = default;
        };

        template <typename T>
        T create_zero_initialized_component() noexcept
        {
            T value;
            std::memset(&value, 0, sizeof(T));
            return value;
        }
    }

    TEST_CASE("Component chunk group hides the component types", "[component_chunk_group]")
    {
        std::array<Component_type_info, 2> const component_type_infos = 
            make_component_type_info_array<Component_a, Component_b>();

        Component_chunk_group const group{component_type_infos, 1, {}, {}};

        CHECK(group.has_component_type(component_type_infos[0].id));
        CHECK(group.has_component_type(component_type_infos[1].id));
        CHECK(!group.has_component_type(get_component_type_id<Component_c>()));
    }

    TEST_CASE("If no chunk is available, a new chunk is created", "[component_chunk_group]")
    {
        constexpr Chunk_group_hash chunk_group_0{0};
        constexpr Chunk_group_hash chunk_group_1{1};

        std::array<Component_type_info, 2> const component_type_infos = 
            make_component_type_info_array<Component_a, Component_b>();

        Component_chunk_group group{component_type_infos, 1, {}, {}};

        CHECK(group.number_of_chunks(chunk_group_0) == 0);

        Component_chunk_group::Index const entity_1_index = group.add_entity(Entity{1}, chunk_group_0);

        CHECK(group.get_entity(chunk_group_0, entity_1_index) == Entity{1});
        CHECK(group.number_of_chunks(chunk_group_0) == 1);

        Component_chunk_group::Index const entity_2_index = group.add_entity(Entity{2}, chunk_group_0);

        CHECK(group.get_entity(chunk_group_0, entity_2_index) == Entity{2});
        CHECK(group.number_of_chunks(chunk_group_0) == 2);
        CHECK(group.number_of_chunks() == 2);
        CHECK(group.number_of_chunks(chunk_group_1) == 0);

        Component_chunk_group::Index const entity_3_index = group.add_entity(Entity{3}, chunk_group_1);

        CHECK(group.get_entity(chunk_group_1, entity_3_index) == Entity{3});
        CHECK(group.number_of_chunks(chunk_group_1) == 1);
        CHECK(group.number_of_chunks() == 3);

        Component_chunk_group::Index const entity_4_index = group.add_entity(Entity{4}, chunk_group_1);

        CHECK(group.get_entity(chunk_group_1, entity_4_index) == Entity{4});
        CHECK(group.number_of_chunks(chunk_group_1) == 2);
        CHECK(group.number_of_chunks() == 4);
    }

    TEST_CASE("If a chunk is available, a new chunk is not created", "[component_chunk_group]")
    {
        constexpr Chunk_group_hash chunk_group_0{0};
        constexpr Chunk_group_hash chunk_group_1{1};

        std::array<Component_type_info, 2> const component_type_infos = 
            make_component_type_info_array<Component_a, Component_b>();

        Component_chunk_group group{component_type_infos, 2, {}, {}};

        CHECK(group.number_of_chunks(chunk_group_0) == 0);

        Component_chunk_group::Index const entity_1_index = group.add_entity(Entity{1}, chunk_group_0);

        CHECK(group.get_entity(chunk_group_0, entity_1_index) == Entity{1});
        CHECK(group.number_of_chunks(chunk_group_0) == 1);

        Component_chunk_group::Index const entity_2_index = group.add_entity(Entity{2}, chunk_group_0);

        CHECK(group.get_entity(chunk_group_0, entity_2_index) == Entity{2});
        CHECK(group.number_of_chunks(chunk_group_0) == 1);

        Component_chunk_group::Index const entity_3_index = group.add_entity(Entity{3}, chunk_group_0);

        CHECK(group.get_entity(chunk_group_0, entity_3_index) == Entity{3});
        CHECK(group.number_of_chunks(chunk_group_0) == 2);
        CHECK(group.number_of_chunks() == 2);
        CHECK(group.number_of_chunks(chunk_group_1) == 0);

        Component_chunk_group::Index const entity_4_index = group.add_entity(Entity{4}, chunk_group_1);

        CHECK(group.get_entity(chunk_group_1, entity_4_index) == Entity{4});
        CHECK(group.number_of_chunks(chunk_group_1) == 1);
        CHECK(group.number_of_chunks() == 3);

        Component_chunk_group::Index const entity_5_index = group.add_entity(Entity{5}, chunk_group_1);

        CHECK(group.get_entity(chunk_group_1, entity_5_index) == Entity{5});
        CHECK(group.number_of_chunks(chunk_group_1) == 1);
        CHECK(group.number_of_chunks() == 3);

        Component_chunk_group::Index const entity_6_index = group.add_entity(Entity{6}, chunk_group_1);

        CHECK(group.get_entity(chunk_group_1, entity_6_index) == Entity{6});
        CHECK(group.number_of_chunks(chunk_group_1) == 2);
        CHECK(group.number_of_chunks() == 4);
    }

    TEST_CASE("An entity is added in the the first non-full chunk", "[component_chunk_group]")
    {
        constexpr Chunk_group_hash chunk_group_0{0};

        std::array<Component_type_info, 2> const component_type_infos = 
            make_component_type_info_array<Component_a, Component_b>();

         Component_chunk_group group{component_type_infos, 2, {}, {}};

         group.add_entity(Entity{0}, chunk_group_0);
         group.add_entity(Entity{1}, chunk_group_0);
         group.add_entity(Entity{2}, chunk_group_0);

         group.remove_entity(chunk_group_0, 2);
         group.remove_entity(chunk_group_0, 1);
         group.remove_entity(chunk_group_0, 0);

         Component_chunk_group::Index const entity_3_index = group.add_entity(Entity{3}, chunk_group_0);

         CHECK(entity_3_index == 0);
         CHECK(group.get_entity(chunk_group_0, entity_3_index) == Entity{3});
    }

    TEST_CASE("Removing an entity results in a swap and pop if element is not the last", "[component_chunk_group]")
    {
        constexpr Chunk_group_hash chunk_group_0{0};

        std::array<Component_type_info, 2> const component_type_infos = 
            make_component_type_info_array<Component_a, Component_b>();

        Component_chunk_group group{component_type_infos, 2, {}, {}};

        SECTION("Remove first element of a group with 1 chunk and 2 elements")
        {
            group.add_entity(Entity{0}, chunk_group_0);
            group.set_component_value(chunk_group_0, 0, Component_a{.value=10});
            group.set_component_value(chunk_group_0, 0, Component_b{.value=20});
        
            group.add_entity(Entity{1}, chunk_group_0);
            group.set_component_value(chunk_group_0, 1, Component_a{.value=11});
            group.set_component_value(chunk_group_0, 1, Component_b{.value=21});
            
            CHECK(group.number_of_entities(chunk_group_0) == 2);
            CHECK(group.get_entity(chunk_group_0, 0) == Entity{0});
            CHECK(group.get_component_value<Component_a>(chunk_group_0, 0) == Component_a{.value=10});
            CHECK(group.get_component_value<Component_b>(chunk_group_0, 0) == Component_b{.value=20});
            CHECK(group.get_entity(chunk_group_0, 1) == Entity{1});
            CHECK(group.get_component_value<Component_a>(chunk_group_0, 1) == Component_a{.value=11});
            CHECK(group.get_component_value<Component_b>(chunk_group_0, 1) == Component_b{.value=21});
            
            std::optional<Component_group_entity_moved> const moved_entity = group.remove_entity(chunk_group_0, 0);

            CHECK(group.number_of_entities(chunk_group_0) == 1);
            CHECK((moved_entity.has_value() && moved_entity->entity == Entity{1}));
            CHECK(group.get_entity(chunk_group_0, 0) == Entity{1});
            CHECK(group.get_component_value<Component_a>(chunk_group_0, 0) == Component_a{.value=11});
            CHECK(group.get_component_value<Component_b>(chunk_group_0, 0) == Component_b{.value=21});
        }

        SECTION("Remove first element of a group with 2 chunks and 3 elements")
        {
            group.add_entity(Entity{0}, chunk_group_0);
            group.set_component_value(chunk_group_0, 0, Component_a{.value=10});
            group.set_component_value(chunk_group_0, 0, Component_b{.value=20});

            group.add_entity(Entity{1}, chunk_group_0);
            group.set_component_value(chunk_group_0, 1, Component_a{.value=11});
            group.set_component_value(chunk_group_0, 1, Component_b{.value=21});

            group.add_entity(Entity{2}, chunk_group_0);
            group.set_component_value(chunk_group_0, 2, Component_a{.value=12});
            group.set_component_value(chunk_group_0, 2, Component_b{.value=22});
            
            CHECK(group.number_of_entities(chunk_group_0) == 3);
            CHECK(group.get_entity(chunk_group_0, 0) == Entity{0});
            CHECK(group.get_component_value<Component_a>(chunk_group_0, 0) == Component_a{.value=10});
            CHECK(group.get_component_value<Component_b>(chunk_group_0, 0) == Component_b{.value=20});
            CHECK(group.get_entity(chunk_group_0, 1) == Entity{1});
            CHECK(group.get_component_value<Component_a>(chunk_group_0, 1) == Component_a{.value=11});
            CHECK(group.get_component_value<Component_b>(chunk_group_0, 1) == Component_b{.value=21});
            CHECK(group.get_entity(chunk_group_0, 2) == Entity{2});
            CHECK(group.get_component_value<Component_a>(chunk_group_0, 2) == Component_a{.value=12});
            CHECK(group.get_component_value<Component_b>(chunk_group_0, 2) == Component_b{.value=22});
            
            std::optional<Component_group_entity_moved> const moved_entity = group.remove_entity(chunk_group_0, 0);

            CHECK(group.number_of_entities(chunk_group_0) == 2);
            CHECK((moved_entity.has_value() && moved_entity->entity == Entity{2}));
            CHECK(group.get_entity(chunk_group_0, 0) == Entity{2});
            CHECK(group.get_component_value<Component_a>(chunk_group_0, 0) == Component_a{.value=12});
            CHECK(group.get_component_value<Component_b>(chunk_group_0, 0) == Component_b{.value=22});
            CHECK(group.get_entity(chunk_group_0, 1) == Entity{1});
            CHECK(group.get_component_value<Component_a>(chunk_group_0, 1) == Component_a{.value=11});
            CHECK(group.get_component_value<Component_b>(chunk_group_0, 1) == Component_b{.value=21});
        }

        SECTION("Remove last element of a group")
        {
            constexpr Chunk_group_hash chunk_group_0{0};

            group.add_entity(Entity{0}, chunk_group_0);
            group.set_component_value(chunk_group_0, 0, Component_a{.value=10});
            group.set_component_value(chunk_group_0, 0, Component_b{.value=20});

            group.add_entity(Entity{1}, chunk_group_0);
            group.set_component_value(chunk_group_0, 1, Component_a{.value=11});
            group.set_component_value(chunk_group_0, 1, Component_b{.value=21});

            group.add_entity(Entity{2}, chunk_group_0);
            group.set_component_value(chunk_group_0, 2, Component_a{.value=12});
            group.set_component_value(chunk_group_0, 2, Component_b{.value=22});
            
            CHECK(group.number_of_entities(chunk_group_0) == 3);
            CHECK(group.get_entity(chunk_group_0, 0) == Entity{0});
            CHECK(group.get_component_value<Component_a>(chunk_group_0, 0) == Component_a{.value=10});
            CHECK(group.get_component_value<Component_b>(chunk_group_0, 0) == Component_b{.value=20});
            CHECK(group.get_entity(chunk_group_0, 1) == Entity{1});
            CHECK(group.get_component_value<Component_a>(chunk_group_0, 1) == Component_a{.value=11});
            CHECK(group.get_component_value<Component_b>(chunk_group_0, 1) == Component_b{.value=21});
            CHECK(group.get_entity(chunk_group_0, 2) == Entity{2});
            CHECK(group.get_component_value<Component_a>(chunk_group_0, 2) == Component_a{.value=12});
            CHECK(group.get_component_value<Component_b>(chunk_group_0, 2) == Component_b{.value=22});
            
            std::optional<Component_group_entity_moved> const moved_entity_0 = group.remove_entity(chunk_group_0, 2);

            CHECK(group.number_of_entities(chunk_group_0) == 2);
            CHECK(!moved_entity_0.has_value());
            CHECK(group.get_entity(chunk_group_0, 0) == Entity{0});
            CHECK(group.get_component_value<Component_a>(chunk_group_0, 0) == Component_a{.value=10});
            CHECK(group.get_component_value<Component_b>(chunk_group_0, 0) == Component_b{.value=20});
            CHECK(group.get_entity(chunk_group_0, 1) == Entity{1});
            CHECK(group.get_component_value<Component_a>(chunk_group_0, 1) == Component_a{.value=11});
            CHECK(group.get_component_value<Component_b>(chunk_group_0, 1) == Component_b{.value=21});

            std::optional<Component_group_entity_moved> const moved_entity_1 = group.remove_entity(chunk_group_0, 1);

            CHECK(group.number_of_entities(chunk_group_0) == 1);
            CHECK(!moved_entity_1.has_value());
            CHECK(group.get_entity(chunk_group_0, 0) == Entity{0});
            CHECK(group.get_component_value<Component_a>(chunk_group_0, 0) == Component_a{.value=10});
            CHECK(group.get_component_value<Component_b>(chunk_group_0, 0) == Component_b{.value=20});

            std::optional<Component_group_entity_moved> const moved_entity_2 = group.remove_entity(chunk_group_0, 0);

            CHECK(group.number_of_entities(chunk_group_0) == 0);
            CHECK(!moved_entity_2.has_value());
        }
    }

    TEST_CASE("When a chunk is empty, it is not destroyed unless shrink_to_fit is called", "[component_chunk_group]")
    {
        constexpr Chunk_group_hash chunk_group_0{0};

        std::array<Component_type_info, 2> const component_type_infos = 
            make_component_type_info_array<Component_a, Component_b>();

        Component_chunk_group group{component_type_infos, 2, {}, {}};

        group.add_entity(Entity{0}, chunk_group_0);
        group.add_entity(Entity{1}, chunk_group_0);
        group.add_entity(Entity{2}, chunk_group_0);

        CHECK(group.number_of_chunks(chunk_group_0) == 2);

        group.remove_entity(chunk_group_0, 0);

        CHECK(group.number_of_chunks(chunk_group_0) == 2);

        group.shrink_to_fit(chunk_group_0);

        CHECK(group.number_of_chunks(chunk_group_0) == 1);
    }

    TEST_CASE("Components are zero-initialized in a component chunk group", "[component_chunk_group]")
    {
        constexpr Chunk_group_hash chunk_group_0{0};

        std::array<Component_type_info, 2> const component_type_infos = 
            make_component_type_info_array<Component_a, Component_b>();

        Component_chunk_group group{component_type_infos, 2, {}, {}};
        Component_chunk_group::Index const entity_0_index = group.add_entity(Entity{1}, chunk_group_0);

        {
            Component_a const expected_value = create_zero_initialized_component<Component_a>();
            Component_a const actual_value = group.get_component_value<Component_a>(chunk_group_0, entity_0_index);

            CHECK(actual_value == expected_value);
        }

        {
            Component_b const expected_value = create_zero_initialized_component<Component_b>();
            Component_b const actual_value = group.get_component_value<Component_b>(chunk_group_0, entity_0_index);

            CHECK(actual_value == expected_value);
        }

        group.set_component_value(chunk_group_0, entity_0_index, Component_a{.value = 1});
        group.set_component_value(chunk_group_0, entity_0_index, Component_b{.value = 1});
        group.remove_entity(chunk_group_0, entity_0_index);

        Component_chunk_group::Index const entity_1_index = group.add_entity(Entity{2}, chunk_group_0);

        {
            Component_a const expected_value = create_zero_initialized_component<Component_a>();
            Component_a const actual_value = group.get_component_value<Component_a>(chunk_group_0, entity_1_index);

            CHECK(actual_value == expected_value);
        }

        {
            Component_b const expected_value = create_zero_initialized_component<Component_b>();
            Component_b const actual_value = group.get_component_value<Component_b>(chunk_group_0, entity_1_index);

            CHECK(actual_value == expected_value);
        }

        Component_chunk_group::Index const entity_2_index = group.add_entity(Entity{3}, chunk_group_0);
        group.set_component_value(chunk_group_0, entity_2_index, Component_a{.value = 1});
        group.set_component_value(chunk_group_0, entity_2_index, Component_b{.value = 1});

        Component_chunk_group::Index const entity_3_index = group.add_entity(Entity{4}, chunk_group_0);

        {
            Component_a const expected_value = create_zero_initialized_component<Component_a>();
            Component_a const actual_value = group.get_component_value<Component_a>(chunk_group_0, entity_3_index);

            CHECK(actual_value == expected_value);
        }

        {
            Component_b const expected_value = create_zero_initialized_component<Component_b>();
            Component_b const actual_value = group.get_component_value<Component_b>(chunk_group_0, entity_3_index);

            CHECK(actual_value == expected_value);
        }

        group.set_component_value(chunk_group_0, entity_3_index, Component_a{.value = 1});
        group.set_component_value(chunk_group_0, entity_3_index, Component_b{.value = 1});
        group.remove_entity(chunk_group_0, entity_3_index);

        Component_chunk_group::Index const entity_4_index = group.add_entity(Entity{5}, chunk_group_0);

        {
            Component_a const expected_value = create_zero_initialized_component<Component_a>();
            Component_a const actual_value = group.get_component_value<Component_a>(chunk_group_0, entity_4_index);

            CHECK(actual_value == expected_value);
        }

        {
            Component_b const expected_value = create_zero_initialized_component<Component_b>();
            Component_b const actual_value = group.get_component_value<Component_b>(chunk_group_0, entity_4_index);

            CHECK(actual_value == expected_value);
        }
    }

    TEST_CASE("Set component values in a component chunk group", "[component_chunk_group]")
    {
        constexpr Chunk_group_hash chunk_group_0{0};
        constexpr Entity entity{ 1 };

        std::array<Component_type_info, 2> const component_type_infos = 
            make_component_type_info_array<Component_a, Component_b>();

        Component_chunk_group group{component_type_infos, 2, {}, {}};
        Component_chunk_group::Index const entity_index = group.add_entity(entity, chunk_group_0);

        {
            constexpr Component_a new_value{ .value = 10 };
            group.set_component_value(chunk_group_0, entity_index, new_value);

            Component_a const actual_value = group.get_component_value<Component_a>(chunk_group_0, entity_index);
            CHECK(actual_value == new_value);
        }

        {
            constexpr Component_b new_value{ .value = 12 };
            group.set_component_value(chunk_group_0, entity_index, new_value);

            Component_b const actual_value = group.get_component_value<Component_b>(chunk_group_0, entity_index);
            CHECK(actual_value == new_value);
        }
    }

// chunk_group_view -> range of elements
// Use case 1:
// G0 [[a0, ..., an] + [b0, ..., bn] -> [c0, ..., cn], ...]
// G1 [[a0, ..., an] + [b0, ..., bn] -> [c0, ..., cn], ...]
//
// Use case 2:
// G0 [[c0, ..., cn], ...] + Shared data
// G1 [[c0, ..., cn], ...] + Shared data
//
// G0 [[a0, ..., an] + [b0, ..., bn] -> [c0, ..., cn], ...] -> chunk range iterator

// Type A (chunk_group + shared_component + type + chunk)
// Iterator a [a0, ..., an]
// Iterator b [b0, ..., bn]
// Iterator c [c0, ..., cn]

// Type B (chunk_group + shared_component + types + chunk)
// Iterator abc0 -> {Iterator a0, Iterator b0, Iterator c0} // Increment at the same time
// Iterator abc1 -> {Iterator a1, Iterator b1, Iterator c1}

// Type C (chunk_group + shared_component + types)
// Iterator abc0n -> [Iterator abc0, ... Iterator abcn]

// Type D (chunk_group)
// Iterator -> [Iterator abc0n0, ..., Iterator abc0nn]

    template<std::random_access_iterator T>
    void is_random_access_iterator(T)
    {
    }

    TEST_CASE("Use component chunk view", "[component_chunk_group]")
    {
        constexpr Chunk_group_hash chunk_group_0{0};
        constexpr Chunk_group_hash chunk_group_1{1};

        std::array<Component_type_info, 2> const component_type_infos = 
            make_component_type_info_array<Component_a, Component_b>();

        Component_chunk_group group{component_type_infos, 2, {}, {}};

        group.add_entity(Entity{1}, chunk_group_0);
        group.set_component_value(chunk_group_0, 0, Component_a{.value=1});
        group.set_component_value(chunk_group_0, 0, Component_b{.value=2});

        
        group.add_entity(Entity{2}, chunk_group_1);
        group.set_component_value(chunk_group_1, 0, Component_a{.value=3});
        group.set_component_value(chunk_group_1, 0, Component_b{.value=4});

        group.add_entity(Entity{3}, chunk_group_1);
        group.set_component_value(chunk_group_1, 1, Component_a{.value=5});
        group.set_component_value(chunk_group_1, 1, Component_b{.value=6});

        group.add_entity(Entity{4}, chunk_group_1);
        group.set_component_value(chunk_group_1, 2, Component_a{.value=7});
        group.set_component_value(chunk_group_1, 2, Component_b{.value=8});

        REQUIRE(group.number_of_chunks(chunk_group_0) == 1);
        REQUIRE(group.number_of_chunks(chunk_group_1) == 2);

        {
            using Iterator = Component_chunk_group::Iterator<Entity>;
            is_random_access_iterator(Iterator{});
        }

        {
            using Iterator_traits = std::iterator_traits<Component_chunk_group::Iterator<Entity>>;
            static_assert(std::is_same_v<Iterator_traits::value_type, Entity>);
        }

        {
            using Iterator_traits = std::iterator_traits<Component_chunk_group::Iterator<Component_a>>;
            static_assert(std::is_same_v<Iterator_traits::value_type, Component_a>);
        }

        {
            using Iterator_traits = std::iterator_traits<Component_chunk_group::Iterator<Component_b>>;
            static_assert(std::is_same_v<Iterator_traits::value_type, Component_b>);
        }

        {
            constexpr std::size_t chunk_index = 0;
            
            auto const view = group.get_view<Entity>(chunk_group_0, chunk_index);
            
            CHECK(std::distance(view.begin(), view.end()) == 1);
            CHECK(*(view.begin() + 0) == Entity{1});
        }

        {
            constexpr std::size_t chunk_index = 0;

            auto const view = group.get_view<Component_a>(chunk_group_0, chunk_index);
            
            CHECK(std::distance(view.begin(), view.end()) == 1);
            CHECK(*(view.begin() + 0) == Component_a{.value=1});
        }

        {
            constexpr std::size_t chunk_index = 0;
            
            auto const view = group.get_view<Component_b>(chunk_group_0, chunk_index);
            
            CHECK(std::distance(view.begin(), view.end()) == 1);
            CHECK(*(view.begin() + 0) == Component_b{.value=2});
        }

        {
            constexpr std::size_t chunk_index = 0;
            
            auto const view = group.get_view<Entity>(chunk_group_1, chunk_index);
            
            CHECK(std::distance(view.begin(), view.end()) == 2);
            CHECK(*(view.begin() + 0) == Entity{2});
            CHECK(*(view.begin() + 1) == Entity{3});
        }

        {
            constexpr std::size_t chunk_index = 0;
            
            auto const view = group.get_view<Component_a>(chunk_group_1, chunk_index);
            
            CHECK(std::distance(view.begin(), view.end()) == 2);
            CHECK(*(view.begin() + 0) == Component_a{.value=3});
            CHECK(*(view.begin() + 1) == Component_a{.value=5});
        }

        {
            constexpr std::size_t chunk_index = 0;
            
            auto const view = group.get_view<Component_b>(chunk_group_1, chunk_index);
            
            CHECK(std::distance(view.begin(), view.end()) == 2);
            CHECK(*(view.begin() + 0) == Component_b{.value=4});
            CHECK(*(view.begin() + 1) == Component_b{.value=6});
        }

        {
            constexpr std::size_t chunk_index = 1;
            
            auto const view = group.get_view<Entity>(chunk_group_1, chunk_index);
            
            CHECK(std::distance(view.begin(), view.end()) == 1);
            CHECK(*(view.begin() + 0) == Entity{4});
        }

        {
            constexpr std::size_t chunk_index = 1;
            
            auto const view = group.get_view<Component_a>(chunk_group_1, chunk_index);
            
            CHECK(std::distance(view.begin(), view.end()) == 1);
            CHECK(*(view.begin() + 0) == Component_a{.value=7});
        }

        {
            constexpr std::size_t chunk_index = 1;
            
            auto const view = group.get_view<Component_b>(chunk_group_1, chunk_index);
            
            CHECK(std::distance(view.begin(), view.end()) == 1);
            CHECK(*(view.begin() + 0) == Component_b{.value=8});
        }

        {
            constexpr std::size_t chunk_index = 0;

            auto const view = group.get_view<Component_b>(chunk_group_1, chunk_index);

            auto const plus_one = [](Component_b const component) -> Component_b
            {
                return {component.value + 1};
            };

            std::transform(view.begin(), view.end(), view.begin(), plus_one);

            CHECK(std::distance(view.begin(), view.end()) == 2);
            CHECK(*(view.begin() + 0) == Component_b{.value=5});
            CHECK(*(view.begin() + 1) == Component_b{.value=7});
        }
    }
}