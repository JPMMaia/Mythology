import maia.ecs.component;
import maia.ecs.component_chunk_group;
import maia.ecs.entity;

import <catch2/catch.hpp>;

import <array>;
import <cstring>;

namespace Maia::ECS::Test
{
    namespace
    {
        template<typename T>
        struct Component_base {};

        template<typename T>
        bool operator==(Component_base<T> const lhs, Component_base<T> const rhs)
        {
            return std::memcmp(&lhs, &rhs, sizeof(T));
        }

        template<typename T>
        bool operator!=(Component_base<T> const lhs, Component_base<T> const rhs)
        {
            return !(lhs == rhs);
        }

        struct Component_a : Component_base<Component_a>
        {
            int value = 1;
        };

        struct Component_b : Component_base<Component_b>
        {
            int value = 2;
        };

        struct Component_c : Component_base<Component_c>
        {
            int value = 3;
        };

        struct Shared_component_d : Component_base<Shared_component_d>
        {
            int value = 4;
        };

        struct Shared_component_e : Component_base<Shared_component_e>
        {
            int value = 4;
        };
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
        std::array<Component_type_info, 2> const component_type_infos = 
            make_component_type_info_array<Component_a, Component_b>();

        Component_chunk_group group{component_type_infos, 1, {}, {}};

        CHECK(group.number_of_chunks() == 0);

        group.add_entity(Entity{0});

        CHECK(group.number_of_chunks() == 1);

        group.add_entity(Entity{1});

        CHECK(group.number_of_chunks() == 2);
    }

    TEST_CASE("If a chunk is available, a new chunk is not created", "[component_chunk_group]")
    {
        std::array<Component_type_info, 2> const component_type_infos = 
            make_component_type_info_array<Component_a, Component_b>();

        Component_chunk_group group{component_type_infos, 2, {}, {}};

        CHECK(group.number_of_chunks() == 0);

        group.add_entity(Entity{0});

        CHECK(group.number_of_chunks() == 1);

        group.add_entity(Entity{1});

        CHECK(group.number_of_chunks() == 1);

        group.add_entity(Entity{2});

        CHECK(group.number_of_chunks() == 2);
    }

    TEST_CASE("Removing an entity results in a swap and pop if element is not the last", "[component_chunk_group]")
    {
        std::array<Component_type_info, 2> const component_type_infos = 
            make_component_type_info_array<Component_a, Component_b>();

        Component_chunk_group group{component_type_infos, 2, {}, {}};

        SECTION("Remove first element of a group with 1 chunk and 2 elements")
        {
            group.add_entity(Entity{0});
            group.add_entity(Entity{1});
            
            CHECK(group.number_of_entities() == 2);
            CHECK(group.get_entity(0) == Entity{0});
            CHECK(group.get_entity(1) == Entity{1});
            
            group.remove_entity(0);

            CHECK(group.number_of_entities() == 1);
            CHECK(group.get_entity(0) == Entity{1});
        }

        SECTION("Remove first element of a group with 2 chunks and 3 elements")
        {
            group.add_entity(Entity{0});
            group.add_entity(Entity{1});
            group.add_entity(Entity{2});
            
            CHECK(group.number_of_entities() == 3);
            CHECK(group.get_entity(0) == Entity{0});
            CHECK(group.get_entity(1) == Entity{1});
            CHECK(group.get_entity(2) == Entity{2});
            
            group.remove_entity(0);

            CHECK(group.number_of_entities() == 2);
            CHECK(group.get_entity(0) == Entity{2});
            CHECK(group.get_entity(1) == Entity{1});
        }

        SECTION("Remove last element of a group")
        {
            group.add_entity(Entity{0});
            group.add_entity(Entity{1});
            group.add_entity(Entity{2});
            
            CHECK(group.number_of_entities() == 3);
            CHECK(group.get_entity(0) == Entity{0});
            CHECK(group.get_entity(1) == Entity{1});
            CHECK(group.get_entity(2) == Entity{2});
            
            group.remove_entity(2);

            CHECK(group.number_of_entities() == 2);
            CHECK(group.get_entity(0) == Entity{0});
            CHECK(group.get_entity(1) == Entity{1});

            group.remove_entity(1);

            CHECK(group.number_of_entities() == 1);
            CHECK(group.get_entity(0) == Entity{0});

            group.remove_entity(0);

            CHECK(group.number_of_entities() == 0);
        }
    }

    TEST_CASE("When a chunk is empty, it is not destroyed unless shrink_to_fit is called", "[component_chunk_group]")
    {
        std::array<Component_type_info, 2> const component_type_infos = 
            make_component_type_info_array<Component_a, Component_b>();

        Component_chunk_group group{component_type_infos, 2, {}, {}};

        group.add_entity(Entity{0});
        group.add_entity(Entity{1});
        group.add_entity(Entity{2});

        CHECK(group.number_of_chunks() == 2);

        group.remove_entity(0);

        CHECK(group.number_of_chunks() == 2);

        group.shrink_to_fit();

        CHECK(group.number_of_chunks() == 1);
    }

    TEST_CASE("Components are initialized with default values in a component chunk group", "[component_chunk_group]")
    {
        constexpr Entity entity{ 1 };

        std::array<Component_type_info, 2> const component_type_infos = 
            make_component_type_info_array<Component_a, Component_b>();

        Component_chunk_group group{component_type_infos, {}, {}};
        group.add_entity(entity);

        {
            constexpr Component_a expected_value{};
            Component_a const actual_value = group.get_component_value<Component_a>(entity);

            CHECK(actual_value == expected_value);
        }

        {
            constexpr Component_b expected_value{};
            Component_b const actual_value = group.get_component_value<Component_b>(entity);

            CHECK(actual_value == expected_value);
        }
    }

    TEST_CASE("Set component values in a component chunk group", "[component_chunk_group]")
    {
        constexpr Entity entity{ 1 };

        std::array<Component_type_info, 2> const component_type_infos = 
            make_component_type_info_array<Component_a, Component_b>();

        Component_chunk_group group{component_type_infos, {}, {}};
        group.add_entity(entity);

        {
            constexpr Component_a new_value{ .value = 10 };
            group.set_component_value(entity, new_value);

            Component_a const actual_value = group.get_component_value<Component_a>(entity);
            CHECK(actual_value == new_value);
        }

        {
            constexpr Component_b new_value{ .value = 12 };
            group.set_component_value(entity, new_value);

            Component_b const actual_value = group.get_component_value<Component_b>(entity);
            CHECK(actual_value == new_value);
        }
    }

    TEST_CASE("A component chunk group hides the shared component types", "[component_chunk_group]")
    {
        std::array<Component_type_info, 2> const component_type_infos = 
            make_component_type_info_array<Component_a, Component_b>();

        Shared_component_type_info const shared_component_type_info =
            make_shared_component_type_info<Shared_component_d>();

        Component_chunk_group const group{component_type_infos, shared_component_type_info, 1, {}, {}};

        CHECK(group.has_shared_component_type(shared_component_type_info.id));
        CHECK(!group.has_shared_component_type(get_shared_component_type_id<Shared_component_type_e>());
    }

    TEST_CASE("Each chunk in a chunk group has a shared component value associated with it", "[component_chunk_group]")
    {
        std::array<Component_type_info, 2> const component_type_infos = 
            make_component_type_info_array<Component_a, Component_b>();

        Shared_component_type_info const shared_component_type_info =
            make_shared_component_type_info<Shared_component_d>();

        Component_chunk_group group{component_type_infos, shared_component_type_info, 2, {}, {}};

        constexpr Shared_component_d d_0{.value=10};
        constexpr Shared_component_d d_1{.value=11};

        CHECK(group.number_of_chunks() == 0);
        CHECK(group.number_of_chunks(d_0) == 0);
        CHECK(group.number_of_chunks(d_1) == 0);

        group.add_entity(Entity{0}, d_0);

        CHECK(group.number_of_chunks() == 1);
        CHECK(group.number_of_chunks(d_0) == 1);
        CHECK(group.number_of_chunks(d_1) == 0);

        group.add_entity(Entity{1}, d_1);

        CHECK(group.number_of_chunks() == 2);
        CHECK(group.number_of_chunks(d_0) == 1);
        CHECK(group.number_of_chunks(d_1) == 1);

        group.add_entity(Entity{2}, d_1);

        CHECK(group.number_of_chunks() == 2);
        CHECK(group.number_of_chunks(d_0) == 1);
        CHECK(group.number_of_chunks(d_1) == 1);

        group.add_entity(Entity{3}, d_1);

        CHECK(group.number_of_chunks() == 3);
        CHECK(group.number_of_chunks(d_0) == 1);
        CHECK(group.number_of_chunks(d_1) == 2);

        group.remove_entity(1);

        CHECK(group.number_of_chunks() == 3);
        CHECK(group.number_of_chunks(d_0) == 1);
        CHECK(group.number_of_chunks(d_1) == 2);

        group.shrink_to_fit();

        CHECK(group.number_of_chunks() == 2);
        CHECK(group.number_of_chunks(d_0) == 1);
        CHECK(group.number_of_chunks(d_1) == 1);

        group.remove_entity(0);
        group.shrink_to_fit();

        CHECK(group.number_of_chunks() == 1);
        CHECK(group.number_of_chunks(d_0) == 0);
        CHECK(group.number_of_chunks(d_1) == 1);

        group.remove_entity(2);
        group.shrink_to_fit();

        CHECK(group.number_of_chunks() == 1);
        CHECK(group.number_of_chunks(d_0) == 1);
        CHECK(group.number_of_chunks(d_1) == 1);

        group.remove_entity(1);
        group.shrink_to_fit();

        CHECK(group.number_of_chunks() == 0);
        CHECK(group.number_of_chunks(d_0) == 0);
        CHECK(group.number_of_chunks(d_1) == 0);
    }

    TEST_CASE("Use views in a component chunk group", "[component_chunk_group]")
    {
        CHECK(true == true);
    }
}