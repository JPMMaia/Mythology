#include <catch2/catch.hpp>

#include <array>
#include <cstring>
#include <iterator>
#include <memory_resource>
#include <ranges>

import maia.ecs.component;
import maia.ecs.component_chunk_group;
import maia.ecs.entity;
import maia.test.debug_resource;

import join_view;

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
            make_sorted_component_type_info_array<Component_a, Component_b>();

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
            make_sorted_component_type_info_array<Component_a, Component_b>();

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
            make_sorted_component_type_info_array<Component_a, Component_b>();

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
            make_sorted_component_type_info_array<Component_a, Component_b>();

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
            make_sorted_component_type_info_array<Component_a, Component_b>();

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
            make_sorted_component_type_info_array<Component_a, Component_b>();

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
            make_sorted_component_type_info_array<Component_a, Component_b>();

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
            make_sorted_component_type_info_array<Component_a, Component_b>();

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

    TEST_CASE("Move entities to a different chunk group", "[component_chunk_group]")
    {
        std::array<Component_type_info, 2> const component_type_infos = 
            make_sorted_component_type_info_array<Component_a, Component_b>();

        Component_chunk_group group{component_type_infos, 2, {}, {}};

        group.add_entity(Entity{1}, Chunk_group_hash{0});
        group.set_component_value(Chunk_group_hash{0}, 0, Component_a{.value=1});
        group.set_component_value(Chunk_group_hash{0}, 0, Component_b{.value=2});

        CHECK(group.number_of_entities(Chunk_group_hash{0}) == 1);

        Entity_move_result const entity_move_result_0 = group.move_entity(Chunk_group_hash{0}, 0, Chunk_group_hash{1});

        CHECK(entity_move_result_0.new_index == 0);
        CHECK(!entity_move_result_0.entity_moved_by_remove.has_value());

        CHECK(group.number_of_entities(Chunk_group_hash{0}) == 0);
        
        CHECK(group.number_of_entities(Chunk_group_hash{1}) == 1);
        CHECK(group.get_component_value<Entity>(Chunk_group_hash{1}, entity_move_result_0.new_index) == Entity{1});
        CHECK(group.get_component_value<Component_a>(Chunk_group_hash{1}, entity_move_result_0.new_index) == Component_a{.value=1});
        CHECK(group.get_component_value<Component_b>(Chunk_group_hash{1}, entity_move_result_0.new_index) == Component_b{.value=2});

        group.add_entity(Entity{2}, Chunk_group_hash{1});
        
        group.add_entity(Entity{3}, Chunk_group_hash{1});
        group.set_component_value(Chunk_group_hash{1}, 2, Component_a{.value=3});
        group.set_component_value(Chunk_group_hash{1}, 2, Component_b{.value=4});

        group.add_entity(Entity{4}, Chunk_group_hash{1});

        group.add_entity(Entity{5}, Chunk_group_hash{0});

        Entity_move_result const entity_move_result_1 = group.move_entity(Chunk_group_hash{1}, 2, Chunk_group_hash{0});

        CHECK(entity_move_result_1.new_index == 1);
        CHECK(entity_move_result_1.entity_moved_by_remove.has_value());
        
        if (entity_move_result_1.entity_moved_by_remove.has_value())
        {
            CHECK(entity_move_result_1.entity_moved_by_remove->entity == Entity{4});
        }

        CHECK(group.number_of_entities(Chunk_group_hash{0}) == 2);
        
        CHECK(group.number_of_entities(Chunk_group_hash{1}) == 3);
        CHECK(group.get_component_value<Entity>(Chunk_group_hash{0}, entity_move_result_1.new_index) == Entity{3});
        CHECK(group.get_component_value<Component_a>(Chunk_group_hash{0}, entity_move_result_1.new_index) == Component_a{.value=3});
        CHECK(group.get_component_value<Component_b>(Chunk_group_hash{0}, entity_move_result_1.new_index) == Component_b{.value=4});
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

    template<std::bidirectional_iterator T>
    void is_bidirectional_iterator(T) noexcept
    {
    }

    template<std::random_access_iterator T>
    void is_random_access_iterator(T) noexcept
    {
    }

    template <std::ranges::viewable_range R>
    void is_viewable_range(R) noexcept
    {
    }

    TEST_CASE("Use component view", "[component_chunk_group]")
    {
        constexpr Chunk_group_hash chunk_group_0{0};
        constexpr Chunk_group_hash chunk_group_1{1};

        std::array<Component_type_info, 2> const component_type_infos = 
            make_sorted_component_type_info_array<Component_a, Component_b>();

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

        is_viewable_range(Component_chunk_view<Entity const>{});
        is_random_access_iterator(Component_iterator<Entity const>{});

        is_viewable_range(Component_chunk_view<Entity>{});
        is_random_access_iterator(Component_iterator<Entity>{});

        Component_chunk_group const& const_group = group;

        {
            Component_chunk_view<Entity const> const view = group.get_view<Entity>(chunk_group_0, 0);
        }

        {
            constexpr std::size_t chunk_index = 0;
            
            Component_chunk_view<Entity const> const view = const_group.get_view<Entity>(chunk_group_0, chunk_index);
            
            CHECK(std::distance(view.begin(), view.end()) == 1);
            CHECK(*(view.begin() + 0) == Entity{1});
        }

        {
            constexpr std::size_t chunk_index = 0;

            auto const view = const_group.get_view<Component_a>(chunk_group_0, chunk_index);
            
            CHECK(std::distance(view.begin(), view.end()) == 1);
            CHECK(*(view.begin() + 0) == Component_a{.value=1});
        }

        {
            constexpr std::size_t chunk_index = 0;
            
            auto const view = const_group.get_view<Component_b>(chunk_group_0, chunk_index);
            
            CHECK(std::distance(view.begin(), view.end()) == 1);
            CHECK(*(view.begin() + 0) == Component_b{.value=2});
        }

        {
            constexpr std::size_t chunk_index = 0;
            
            auto const view = const_group.get_view<Entity>(chunk_group_1, chunk_index);
            
            CHECK(std::distance(view.begin(), view.end()) == 2);
            CHECK(*(view.begin() + 0) == Entity{2});
            CHECK(*(view.begin() + 1) == Entity{3});
        }

        {
            constexpr std::size_t chunk_index = 0;
            
            auto const view = const_group.get_view<Component_a>(chunk_group_1, chunk_index);
            
            CHECK(std::distance(view.begin(), view.end()) == 2);
            CHECK(*(view.begin() + 0) == Component_a{.value=3});
            CHECK(*(view.begin() + 1) == Component_a{.value=5});
        }

        {
            constexpr std::size_t chunk_index = 0;
            
            auto const view = const_group.get_view<Component_b>(chunk_group_1, chunk_index);
            
            CHECK(std::distance(view.begin(), view.end()) == 2);
            CHECK(*(view.begin() + 0) == Component_b{.value=4});
            CHECK(*(view.begin() + 1) == Component_b{.value=6});
        }

        {
            constexpr std::size_t chunk_index = 1;
            
            auto const view = const_group.get_view<Entity>(chunk_group_1, chunk_index);
            
            CHECK(std::distance(view.begin(), view.end()) == 1);
            CHECK(*(view.begin() + 0) == Entity{4});
        }

        {
            constexpr std::size_t chunk_index = 1;
            
            auto const view = const_group.get_view<Component_a>(chunk_group_1, chunk_index);
            
            CHECK(std::distance(view.begin(), view.end()) == 1);
            CHECK(*(view.begin() + 0) == Component_a{.value=7});
        }

        {
            constexpr std::size_t chunk_index = 1;
            
            auto const view = const_group.get_view<Component_b>(chunk_group_1, chunk_index);
            
            CHECK(std::distance(view.begin(), view.end()) == 1);
            CHECK(*(view.begin() + 0) == Component_b{.value=8});
        }

        {
            constexpr std::size_t chunk_index = 0;

            auto const view = group.get_view<Component_b>(chunk_group_1, chunk_index);

            auto const plus_one = [](Component_view<Component_b> const component_view) -> void
            {
                Component_b component = component_view.read();
                component.value += 1;
                component_view.write(component);
            };

            std::for_each(view.begin(), view.end(), plus_one);

            CHECK(std::distance(view.begin(), view.end()) == 2);
            CHECK(*(view.begin() + 0) == Component_b{.value=5});
            CHECK(*(view.begin() + 1) == Component_b{.value=7});
        }

        {
            constexpr std::size_t chunk_index = 0;
            
            auto const view = const_group.get_view<Component_a>(chunk_group_1, chunk_index);

            auto const iterator = view.begin();
            
            CHECK(std::distance(view.begin(), view.end()) == 2);
            CHECK(iterator[0] == Component_a{.value=3});
            CHECK(iterator[1] == Component_a{.value=5});
        }

        {
            auto const view = const_group.get_view<Entity>(Chunk_group_hash{2}, 0);
            
            CHECK(std::distance(view.begin(), view.end()) == 0);
        }

        {
            auto const view = const_group.get_view<Entity>(Chunk_group_hash{0}, 5);
            
            CHECK(std::distance(view.begin(), view.end()) == 0);
        }

        constexpr Chunk_group_hash chunk_group_2{2};
        group.add_entity(Entity{5}, chunk_group_2);
        group.remove_entity(chunk_group_2, 0);

        {
            auto const view = const_group.get_view<Entity>(chunk_group_2, 0);
            
            CHECK(std::distance(view.begin(), view.end()) == 0);
        }

        group.shrink_to_fit(chunk_group_2);

        {
            auto const view = const_group.get_view<Entity>(chunk_group_2, 0);
            
            CHECK(std::distance(view.begin(), view.end()) == 0);
        }
    }

    TEST_CASE("Use chunk group view", "[component_chunk_group]")
    {
        constexpr Chunk_group_hash chunk_group_0{0};
        constexpr Chunk_group_hash chunk_group_1{1};

        std::array<Component_type_info, 2> const component_type_infos = 
            make_sorted_component_type_info_array<Component_a, Component_b>();

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

        is_viewable_range(Component_chunk_group_view<Entity const>{});
        is_bidirectional_iterator(Component_chunk_iterator<Entity const>{});

        is_viewable_range(Component_chunk_group_view<Entity>{});
        is_bidirectional_iterator(Component_chunk_iterator<Entity>{});

        Component_chunk_group const& const_group = group;

        {
            Component_chunk_group_view<Entity const> const view = group.get_view<Entity>(chunk_group_0);
        }

        {
            auto const view = const_group.get_view<Component_a>(chunk_group_0) | views::join;
            
            CHECK(std::distance(view.begin(), view.end()) == 1);

            auto iterator = view.begin();
            CHECK(*iterator++ == Component_a{.value=1});
        }

        {
            auto const view = const_group.get_view<Component_b>(chunk_group_0) | views::join;
            
            CHECK(std::distance(view.begin(), view.end()) == 1);

            auto iterator = view.begin();
            CHECK(*iterator++ == Component_b{.value=2});
        }

        {
            auto const view = const_group.get_view<Entity>(chunk_group_1) | views::join;
            
            CHECK(std::distance(view.begin(), view.end()) == 3);

            auto iterator = view.begin();
            CHECK(*iterator++ == Entity{2});
            CHECK(*iterator++ == Entity{3});
            CHECK(*iterator++ == Entity{4});
        }

        {
            auto const view = const_group.get_view<Component_a>(chunk_group_1) | views::join;
            
            CHECK(std::distance(view.begin(), view.end()) == 3);

            auto iterator = view.begin();
            CHECK(*iterator++ == Component_a{.value=3});
            CHECK(*iterator++ == Component_a{.value=5});
            CHECK(*iterator++ == Component_a{.value=7});
        }

        {
            auto const view = const_group.get_view<Component_b>(chunk_group_1) | views::join;
            
            CHECK(std::distance(view.begin(), view.end()) == 3);

            auto iterator = view.begin();
            CHECK(*iterator++ == Component_b{.value=4});
            CHECK(*iterator++ == Component_b{.value=6});
            CHECK(*iterator++ == Component_b{.value=8});
        }

        {
            auto const view = group.get_view<Component_b>(chunk_group_1) | views::join;

            auto const plus_one = [](Component_view<Component_b> const component_view) -> void
            {
                Component_b component = component_view.read();
                component.value += 1;
                component_view.write(component);
            };

            std::for_each(view.begin(), view.end(), plus_one);

            CHECK(std::distance(view.begin(), view.end()) == 3);

            auto iterator = view.begin();
            CHECK(*iterator++ == Component_b{.value=5});
            CHECK(*iterator++ == Component_b{.value=7});
            CHECK(*iterator++ == Component_b{.value=9});
        }

        {
            auto const view = const_group.get_view<Entity>(Chunk_group_hash{2}) | views::join;
            
            CHECK(std::distance(view.begin(), view.end()) == 0);
        }

        constexpr Chunk_group_hash chunk_group_2{2};
        group.add_entity(Entity{5}, chunk_group_2);
        group.remove_entity(chunk_group_2, 0);

        {
            auto const view = const_group.get_view<Entity>(chunk_group_2) | views::join;
            
            CHECK(std::distance(view.begin(), view.end()) == 0);
        }

        group.shrink_to_fit(chunk_group_2);

        {
            auto const view = const_group.get_view<Entity>(chunk_group_2) | views::join;
            
            CHECK(std::distance(view.begin(), view.end()) == 0);
        }
    }

    TEST_CASE("Use view for all chunk groups", "[component_chunk_group]")
    {
        constexpr Chunk_group_hash chunk_group_0{0};
        constexpr Chunk_group_hash chunk_group_1{1};

        std::array<Component_type_info, 2> const component_type_infos = 
            make_sorted_component_type_info_array<Component_a, Component_b>();

        Component_chunk_group group{component_type_infos, 2, {}, {}};

        {
            auto const view = group.get_view<Entity>() | views::join | views::join;
            
            CHECK(std::distance(view.begin(), view.end()) == 0);
        }

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

        is_viewable_range(Component_chunk_group_all_view<Entity const>{});
        is_bidirectional_iterator(Component_chunk_group_iterator<Entity const>{});

        is_viewable_range(Component_chunk_group_all_view<Entity>{});
        is_bidirectional_iterator(Component_chunk_group_iterator<Entity>{});

        Component_chunk_group const& const_group = group;

        {
            Component_chunk_group_all_view<Entity const> const view = group.get_view<Entity>();
        }

        {
            auto const view = const_group.get_view<Entity>() | views::join | views::join;
            
            CHECK(std::distance(view.begin(), view.end()) == 4);

            std::vector<Entity> const actual{view.begin(), view.end()};
            
            std::vector<Entity> const expected
            {
                {1},
                {2},
                {3},
                {4},
            };
            
            CHECK_THAT(actual, Catch::Matchers::UnorderedEquals(expected));
        }

        {
            auto const view = const_group.get_view<Component_a>() | views::join | views::join;
            
            CHECK(std::distance(view.begin(), view.end()) == 4);

            std::vector<Component_a> const actual{view.begin(), view.end()};
            
            std::vector<Component_a> const expected
            {
                {1},
                {3},
                {5},
                {7},
            };
            
            CHECK_THAT(actual, Catch::Matchers::UnorderedEquals(expected));
        }

        {
            auto const view = const_group.get_view<Component_b>() | views::join | views::join;
            
            CHECK(std::distance(view.begin(), view.end()) == 4);

            std::vector<Component_b> const actual{view.begin(), view.end()};
            
            std::vector<Component_b> const expected
            {
                {2},
                {4},
                {6},
                {8},
            };
            
            CHECK_THAT(actual, Catch::Matchers::UnorderedEquals(expected));
        }

        {
            auto const view = group.get_view<Component_b>() | views::join | views::join;

            auto const plus_one = [](Component_view<Component_b> const component_view) -> void
            {
                Component_b component = component_view.read();
                component.value += 1;
                component_view.write(component);
            };

            std::for_each(view.begin(), view.end(), plus_one);

            CHECK(std::distance(view.begin(), view.end()) == 4);

            std::vector<Component_b> const actual{view.begin(), view.end()};
            
            std::vector<Component_b> const expected
            {
                {3},
                {5},
                {7},
                {9},
            };
            
            CHECK_THAT(actual, Catch::Matchers::UnorderedEquals(expected));
        }
    }

    TEST_CASE("Use component tuple iterators", "[component_chunk_group]")
    {
        constexpr Chunk_group_hash chunk_group_0{0};
        constexpr Chunk_group_hash chunk_group_1{1};

        std::array<Component_type_info, 2> const component_type_infos = 
            make_sorted_component_type_info_array<Component_a, Component_b>();

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

        Component_chunk_group const& const_group = group;

        {
            constexpr std::size_t chunk_index = 0;
            
            auto const view = const_group.get_view<Entity, Component_a, Component_b>(chunk_group_1, chunk_index);
            
            CHECK(std::distance(view.begin(), view.end()) == 2);

            auto iterator = view.begin();

            CHECK(*iterator++ == std::make_tuple(Entity{2}, Component_a{3}, Component_b{4}));
            CHECK(*iterator++ == std::make_tuple(Entity{3}, Component_a{5}, Component_b{6}));
        }

        {
            constexpr std::size_t chunk_index = 0;
            
            auto const view = const_group.get_view<Entity, Component_a, Component_b>(chunk_group_1, chunk_index);
            
            CHECK(std::distance(view.begin(), view.end()) == 2);

            auto const iterator = view.begin();

            CHECK(iterator[0] == std::make_tuple(Entity{2}, Component_a{3}, Component_b{4}));
            CHECK(iterator[1] == std::make_tuple(Entity{3}, Component_a{5}, Component_b{6}));
        }

        {
            auto const view = const_group.get_view<Entity, Component_a, Component_b>(chunk_group_1) | views::join;
            
            CHECK(std::distance(view.begin(), view.end()) == 3);

            auto iterator = view.begin();

            CHECK(*iterator++ == std::make_tuple(Entity{2}, Component_a{3}, Component_b{4}));
            CHECK(*iterator++ == std::make_tuple(Entity{3}, Component_a{5}, Component_b{6}));
            CHECK(*iterator++ == std::make_tuple(Entity{4}, Component_a{7}, Component_b{8}));
        }

        {
            auto const view = const_group.get_view<Entity, Component_a, Component_b>() | views::join | views::join;
            
            CHECK(std::distance(view.begin(), view.end()) == 4);

            std::vector<std::tuple<Entity, Component_a, Component_b>> const actual{view.begin(), view.end()};
            
            std::vector<std::tuple<Entity, Component_a, Component_b>> const expected
            {
                std::make_tuple(Entity{1}, Component_a{1}, Component_b{2}),
                std::make_tuple(Entity{2}, Component_a{3}, Component_b{4}),
                std::make_tuple(Entity{3}, Component_a{5}, Component_b{6}),
                std::make_tuple(Entity{4}, Component_a{7}, Component_b{8}),
            };
            
            CHECK_THAT(actual, Catch::Matchers::UnorderedEquals(expected));
        }

        {
            auto const view = group.get_view<Entity, Component_a, Component_b>() | views::join | views::join;

            auto const plus_one = [](
                Component_view<Entity const> const entity_view,
                Component_view<Component_a> const component_a_view,
                Component_view<Component_b> const component_b_view
            ) -> void
            {
                {                    
                    Component_a component = component_a_view.read();
                    component.value += 1;
                    
                    component_a_view.write(component);
                }

                {                    
                    Component_b component = component_b_view.read();
                    component.value += 1;
                    
                    component_b_view.write(component);
                }
            };

            std::for_each(view.begin(), view.end(), call_with_tuple_arguments(plus_one));

            CHECK(std::distance(view.begin(), view.end()) == 4);

            std::vector<std::tuple<Entity, Component_a, Component_b>> const actual{view.begin(), view.end()};
            
            std::vector<std::tuple<Entity, Component_a, Component_b>> const expected
            {
                std::make_tuple(Entity{1}, Component_a{2}, Component_b{3}),
                std::make_tuple(Entity{2}, Component_a{4}, Component_b{5}),
                std::make_tuple(Entity{3}, Component_a{6}, Component_b{7}),
                std::make_tuple(Entity{4}, Component_a{8}, Component_b{9}),
            };
            
            CHECK_THAT(actual, Catch::Matchers::UnorderedEquals(expected));
        }
    }

    TEST_CASE("Use generic allocator for component chunks", "[component_chunk_group]")
    {
        constexpr std::array<Component_type_size, 2> component_type_sizes
        {
            sizeof(Component_a),
            sizeof(Component_b)
        };

        constexpr std::array<std::size_t, 3> number_of_entities_per_group = {20, 5, 15};
        constexpr std::size_t number_of_entities_per_chunk = 10;

        constexpr std::size_t required_memory_size = Component_chunk_group::get_required_generic_memory_size(
            number_of_entities_per_group,
            number_of_entities_per_chunk,
            component_type_sizes
        );

        std::array<std::byte, required_memory_size> buffer_storage;
        std::pmr::monotonic_buffer_resource buffer_resource{buffer_storage.data(), buffer_storage.size(), std::pmr::null_memory_resource()};
        std::pmr::polymorphic_allocator<> generic_allocator{&buffer_resource};

        std::array<Component_type_info, 2> const component_type_infos = 
            make_sorted_component_type_info_array<Component_a, Component_b>();

        auto const add_all_entities = [=](Component_chunk_group& groups) -> void
        {
            for (std::size_t group_index = 0; group_index < number_of_entities_per_group.size(); ++group_index)
            {
                Chunk_group_hash const group_hash = {group_index};
                std::size_t const number_entities_in_group = number_of_entities_per_group[group_index];

                for (std::size_t entity_index = 0; entity_index < number_entities_in_group; ++entity_index)
                {
                    Entity const entity = {static_cast<Entity::Integral_type>(entity_index)};
                    
                    groups.add_entity(entity, group_hash);
                }
            }
        };

        auto const remove_all_entities = [=](Component_chunk_group& groups) -> void
        {
            for (std::size_t group_index = 0; group_index < number_of_entities_per_group.size(); ++group_index)
            {
                Chunk_group_hash const group_hash = {group_index};
                std::size_t const number_entities_in_group = number_of_entities_per_group[group_index];

                for (std::size_t entity_index = 0; entity_index < number_entities_in_group; ++entity_index)
                {
                    groups.remove_entity(group_hash, 0);
                }
            }
        };

        auto const add_and_remove_entities = [&]() -> void
        {
            constexpr std::array<Chunk_group_hash, 3> chunk_group_hashes
            {
                Chunk_group_hash{0}, Chunk_group_hash{1}, Chunk_group_hash{2}
            };

            Component_chunk_group groups
            {
                component_type_infos,
                number_of_entities_per_chunk,
                chunk_group_hashes,
                number_of_entities_per_group,
                {},
                generic_allocator
            };

            for (std::size_t i = 0; i < 100; ++i)
            {
                add_all_entities(groups);
                remove_all_entities(groups);
            }
        };

        CHECK_NOTHROW(add_and_remove_entities());
    }

    TEST_CASE("Component_chunk_group component iteration time", "[component_chunk_group][benchmark]")
    {
        constexpr std::uint32_t number_of_entities = 1000000;

        constexpr Chunk_group_hash chunk_group_0{0};

        std::array<Component_type_info, 2> const component_type_infos = 
            make_sorted_component_type_info_array<Component_a, Component_b>();

        constexpr std::size_t number_of_entites_per_chunk = 16*1024 / sizeof(Component_a) + sizeof(Component_b) + sizeof(Entity);
        Component_chunk_group group{component_type_infos, number_of_entites_per_chunk, {}, {}};

        for (std::uint32_t entity_index = 0; entity_index < number_of_entities; ++entity_index)
        {
            Entity const new_entity{entity_index};
            Component_a const new_component_a{static_cast<int>(entity_index)};
            Component_b const new_component_b{static_cast<int>(entity_index)};

            Component_chunk_group::Index const index = group.add_entity(new_entity, chunk_group_0);
            group.set_component_value(chunk_group_0, index, new_component_a);
            group.set_component_value(chunk_group_0, index, new_component_b);
        }

        BENCHMARK("Component_chunk_group component iteration time with for loop only")
        {
            std::size_t dummy = 0;

            for (auto const& chunk_group : group.get_chunk_groups())
            {
                for (auto const& chunk : chunk_group.second.chunks)
                {
                    for (Component_view<Component_a const> const component_view : group.get_view<Component_a>(chunk_group.second, chunk))
                    {
                        Component_a const component = component_view.read();
                        dummy += component.value;
                    }
                }
            }

            return dummy;
        };

        BENCHMARK("Component_chunk_group component iteration time with view and for loop")
        {
            std::size_t dummy = 0;

            auto const view = std::as_const(group).get_view<Component_a>();

            for (auto const& group_view : view)
            {
                for (auto const& chunk_view : group_view)
                {
                    for (Component_view<Component_a const> const component_view : chunk_view)
                    {
                        Component_a const component = component_view.read();
                        dummy += component.value;
                    }
                }
            }

            return dummy;
        };

        BENCHMARK("Component_chunk_group component iteration time with join")
        {
            std::size_t dummy = 0;

            auto const view = group.get_view<Component_a>() | views::join | views::join;

            for (Component_view<Component_a> const component_view : view)
            {
                Component_a const component = component_view.read();
                dummy += component.value;
            }

            return dummy;
        };
    }
}