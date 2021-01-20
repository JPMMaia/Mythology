import maia.ecs.archetype;
import maia.ecs.component;
import maia.ecs.entity_manager;
import maia.ecs.shared_component;

import <catch2/catch.hpp>;

import <algorithm>;
import <array>;
import <concepts>;
import <cstring>;
import <ostream>;
import <ranges>;
import <span>;
import <vector>;
import <utility>;

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
            char value;

            auto operator<=>(Component_a const&) const noexcept = default;
        };

        struct Component_b
        {
            int value = 3;

            auto operator<=>(Component_b const&) const noexcept = default;
        };

        struct Component_c
        {
            double value;

            auto operator<=>(Component_c const&) const noexcept = default;
        };

        struct Component_d
        {
            int value;

            auto operator<=>(Component_d const&) const noexcept = default;
        };

        struct Shared_component_e
        {
            int value;

            auto operator<=>(Shared_component_e const&) const noexcept = default;
        };

        struct Shared_component_f
        {
            float value;

            auto operator<=>(Shared_component_f const&) const noexcept = default;
        };

        template<typename Component_t>
        bool contains_archetype_which_has_component(std::span<Archetype const> const archetypes) noexcept
        {
            auto const has_component_predicate = [](Archetype const& archetype) -> bool
            {
                return archetype.has_component(get_component_type_id<Component_t>());
            };

            auto const archetype_it = std::find_if(
                archetypes.begin(),
                archetypes.end(),
                has_component_predicate
            );

            return archetype_it != archetypes.end();
        }

        template<typename Shared_component_t>
        bool contains_archetype_which_has_shared_component(std::span<Archetype const> const archetypes) noexcept
        {
            auto const has_shared_component_predicate = [](Archetype const& archetype) -> bool
            {
                return archetype.has_shared_component(get_shared_component_type_id<Shared_component_t>());
            };

            auto const archetype_it = std::find_if(
                archetypes.begin(),
                archetypes.end(),
                has_shared_component_predicate
            );

            return archetype_it != archetypes.end();
        }

        /*template<typename Shared_component_t>
        bool contains_shared_component(std::span<Component_chunk_view const> const views, Shared_component_t const& shared_component) noexcept
        {
            auto const contains_shared_component_predicate = [&shared_component](Component_chunk_view const view) -> bool
            {
                return view.get_shared_component<Shared_component_t>().value == shared_component.value;
            };

            auto const shared_component_it = std::find_if(
                views.begin(),
                views.end(),
                contains_shared_component_predicate
            );

            CHECK(shared_component_it != views.end());
        }*/

        template<typename Container_t, typename Value_t>
        bool contains(Container_t const& container, Value_t const& value) noexcept
        {
            return std::find(container.begin(), container.end(), value) != container.end();
        }

        template <typename T>
        T create_zero_initialized_component() noexcept
        {
            T value;
            std::memset(&value, 0, sizeof(T));
            return value;
        }
    }

    TEST_CASE("An archetype is an unordered set of component types", "[entity_manager]")
    {
        std::array<Component_type_info, 2> const archetype_ab_component_type_infos =
            make_sorted_component_type_info_array<Component_a, Component_b>();

        Archetype const archetype_ab{archetype_ab_component_type_infos, std::nullopt, {}};

        CHECK(archetype_ab.has_component(get_component_type_id<Component_a>()) == true);
        CHECK(archetype_ab.has_component(get_component_type_id<Component_b>()) == true);
        CHECK(archetype_ab.has_component(get_component_type_id<Component_c>()) == false);

        {
            Archetype const archetype_ab_clone{archetype_ab_component_type_infos, std::nullopt, {}};
            CHECK(archetype_ab == archetype_ab_clone);
        }

        std::array<Component_type_info, 2> const archetype_ba_component_type_infos =
            make_sorted_component_type_info_array<Component_b, Component_a>();

        Archetype const archetype_ba{archetype_ba_component_type_infos, std::nullopt, {}};

        CHECK(archetype_ba.has_component(get_component_type_id<Component_a>()) == true);
        CHECK(archetype_ba.has_component(get_component_type_id<Component_b>()) == true);
        CHECK(archetype_ba.has_component(get_component_type_id<Component_c>()) == false);

        CHECK(archetype_ab == archetype_ba);

        std::array<Component_type_info, 1> const archetype_a_component_type_infos =
            make_sorted_component_type_info_array<Component_a>();

        Archetype const archetype_a{archetype_a_component_type_infos, std::nullopt, {}};

        CHECK(archetype_ab != archetype_a);
        CHECK(archetype_ba != archetype_a);
    }

    TEST_CASE("Archetypes can have a single shared component type", "[entity_manager]")
    {
        {
            std::array<Component_type_info, 2> const archetype_ab_component_type_infos =
                make_sorted_component_type_info_array<Component_a, Component_b>();

            Archetype const archetype_ab{archetype_ab_component_type_infos, std::nullopt, {}};


            CHECK(archetype_ab.has_shared_component() == false);
            CHECK(archetype_ab.has_shared_component(get_shared_component_type_id<Shared_component_e>()) == false);
            CHECK(archetype_ab.has_shared_component(get_shared_component_type_id<Shared_component_f>()) == false);
        }

        {
            Shared_component_type_ID const shared_component_e_type_id =
                get_shared_component_type_id<Shared_component_e>();

            std::array<Component_type_info, 1> const archetype_ae_component_type_infos =
                make_sorted_component_type_info_array<Component_a>();

            Archetype const archetype_ae{archetype_ae_component_type_infos, shared_component_e_type_id, {}};


            CHECK(archetype_ae.has_shared_component() == true);
            CHECK(archetype_ae.has_shared_component(get_shared_component_type_id<Shared_component_e>()) == true);
            CHECK(archetype_ae.has_shared_component(get_shared_component_type_id<Shared_component_f>()) == false);
        }

        {
            Shared_component_type_ID const shared_component_e_type_id =
                get_shared_component_type_id<Shared_component_e>();

            Archetype const archetype_e{{}, shared_component_e_type_id, {}};

            Shared_component_type_ID const shared_component_f_type_id =
                get_shared_component_type_id<Shared_component_f>();

            Archetype const archetype_f{{}, shared_component_f_type_id, {}};


            CHECK(shared_component_e_type_id != shared_component_f_type_id);
        }
    }

    TEST_CASE("An entity is a tag that identifies components that belong to the same object", "[entity_manager]")
    {
        Entity_manager entity_manager{};

        std::array<Component_type_info, 2> const archetype_ab_component_type_infos =
            make_sorted_component_type_info_array<Component_a, Component_b>();

        Archetype const archetype_ab{archetype_ab_component_type_infos, std::nullopt, {}};

        Entity const entity_0 = entity_manager.create_entity(archetype_ab);

        constexpr Component_a entity_0_a{ .value = 1 };
        entity_manager.set_component_value(entity_0, entity_0_a);

        constexpr Component_b entity_0_b{ .value = 2 };
        entity_manager.set_component_value(entity_0, entity_0_b);

        Entity const entity_1 = entity_manager.create_entity(archetype_ab);

        constexpr Component_a entity_1_a{ .value = 3 };
        entity_manager.set_component_value(entity_1, entity_1_a);

        constexpr Component_b entity_1_b{ .value = 4 };
        entity_manager.set_component_value(entity_1, entity_1_b);


        CHECK(entity_0 != entity_1);

        {
            Component_a const actual_entity_0_a = entity_manager.get_component_value<Component_a>(entity_0);
            CHECK(actual_entity_0_a == entity_0_a);
        }

        {
            Component_b const actual_entity_0_b = entity_manager.get_component_value<Component_b>(entity_0);
            CHECK(actual_entity_0_b == entity_0_b);
        }

        entity_manager.destroy_entity(entity_0);

        {
            Component_a const actual_entity_1_a = entity_manager.get_component_value<Component_a>(entity_1);
            CHECK(actual_entity_1_a == entity_1_a);
        }

        {
            Component_b const actual_entity_1_b = entity_manager.get_component_value<Component_b>(entity_1);
            CHECK(actual_entity_1_b == entity_1_b);
        }

        entity_manager.destroy_entity(entity_1);
    }

    TEST_CASE("When an entity is created, its components are zero-initialized", "[entity_manager]")
    {
        Entity_manager entity_manager{};

        std::array<Component_type_info, 1> const archetype_b_component_type_infos =
            make_sorted_component_type_info_array<Component_b>();

        Archetype const archetype_b{archetype_b_component_type_infos, std::nullopt, {}};

        Entity const entity_0 = entity_manager.create_entity(archetype_b);

        Component_b const initialized_value = entity_manager.get_component_value<Component_b>(entity_0);

        Component_b const zero_initialized_value = create_zero_initialized_component<Component_b>();
        CHECK(initialized_value == zero_initialized_value);
    }

    TEST_CASE("New archetypes can be created by adding or removing components to an entity", "[.entity_manager]")
    {
        Entity_manager entity_manager{};

        std::array<Component_type_info, 1> const archetype_a_component_type_infos =
            make_sorted_component_type_info_array<Component_a>();

        Archetype const archetype_a{archetype_a_component_type_infos, std::nullopt, {}};

        {
            std::span<Archetype const> const archetypes = entity_manager.get_archetypes();

            CHECK(archetypes.size() == 0);
        }

        Entity const entity_0 = entity_manager.create_entity(archetype_a);
        Entity const entity_1 = entity_manager.create_entity(archetype_a);

        {
            std::span<Archetype const> const archetypes = entity_manager.get_archetypes();

            CHECK(archetypes.size() == 1);
            CHECK(contains(archetypes, archetype_a));
            CHECK(contains_archetype_which_has_component<Component_a>(archetypes));
            CHECK(!contains_archetype_which_has_component<Component_b>(archetypes));
        }

        entity_manager.add_component_type<Component_b>(entity_0);

        {
            std::span<Archetype const> const archetypes = entity_manager.get_archetypes();

            CHECK(archetypes.size() == 2);
            CHECK(contains(archetypes, archetype_a));
            CHECK(contains_archetype_which_has_component<Component_a>(archetypes));
            CHECK(contains_archetype_which_has_component<Component_b>(archetypes));
        }

        entity_manager.add_component_type<Component_b>(entity_1);

        {
            std::span<Archetype const> const archetypes = entity_manager.get_archetypes();

            CHECK(archetypes.size() == 1);
            CHECK(!contains(archetypes, archetype_a));
            CHECK(contains_archetype_which_has_component<Component_a>(archetypes));
            CHECK(contains_archetype_which_has_component<Component_b>(archetypes));
        }

        entity_manager.remove_component_type<Component_a>(entity_0);

        {
            std::span<Archetype const> const archetypes = entity_manager.get_archetypes();

            CHECK(archetypes.size() == 2);
            CHECK(contains_archetype_which_has_component<Component_a>(archetypes));
            CHECK(contains_archetype_which_has_component<Component_b>(archetypes));
        }

        entity_manager.remove_component_type<Component_a>(entity_1);

        {
            std::span<Archetype const> const archetypes = entity_manager.get_archetypes();

            CHECK(archetypes.size() == 1);
            CHECK(!contains_archetype_which_has_component<Component_a>(archetypes));
            CHECK(contains_archetype_which_has_component<Component_b>(archetypes));
            CHECK(!contains_archetype_which_has_shared_component<Shared_component_e>(archetypes));
        }

        Shared_component_e constexpr shared_component_e_0{ .value = 1 };
        entity_manager.add_shared_component_type<Shared_component_e>(entity_1, shared_component_e_0);

        {
            std::span<Archetype const> const archetypes = entity_manager.get_archetypes();

            CHECK(archetypes.size() == 2);
            CHECK(!contains_archetype_which_has_component<Component_a>(archetypes));
            CHECK(contains_archetype_which_has_component<Component_b>(archetypes));
            CHECK(contains_archetype_which_has_shared_component<Shared_component_e>(archetypes));
        }

        entity_manager.remove_shared_component_type<Shared_component_e>(entity_1);

        {
            std::span<Archetype const> const archetypes = entity_manager.get_archetypes();

            CHECK(archetypes.size() == 1);
            CHECK(!contains_archetype_which_has_component<Component_a>(archetypes));
            CHECK(contains_archetype_which_has_component<Component_b>(archetypes));
            CHECK(!contains_archetype_which_has_shared_component<Shared_component_e>(archetypes));
        }

        entity_manager.destroy_entity(entity_1);
        entity_manager.destroy_entity(entity_0);

        {
            std::span<Archetype const> const archetypes = entity_manager.get_archetypes();

            CHECK(archetypes.size() == 0);
        }
    }

    TEST_CASE("A shared component is a component that can be shared by multiple entities", "[entity_manager]")
    {
        constexpr Shared_component_e shared_component_e1{ .value = 1 };
        constexpr Shared_component_key shared_component_e1_key = 1;

        Entity_manager entity_manager{};

        Shared_component_type_ID const shared_component_e_type_id =
            get_shared_component_type_id<Shared_component_e>();

        Archetype const archetype_e{{}, shared_component_e_type_id, {}};

        entity_manager.set_shared_component(shared_component_e1_key, shared_component_e1);

        {
            Shared_component_e const& actual_value = entity_manager.get_shared_component<Shared_component_e>(shared_component_e1_key);

            CHECK(actual_value == shared_component_e1);
        }

        Entity const entity_0 = entity_manager.create_entity(archetype_e, shared_component_e1_key);
        Entity const entity_1 = entity_manager.create_entity(archetype_e, shared_component_e1_key);

        {
            Shared_component_e const& actual_value = entity_manager.get_shared_component<Shared_component_e>(entity_0);
            CHECK(actual_value == shared_component_e1);
        }

        {
            Shared_component_e const& actual_value = entity_manager.get_shared_component<Shared_component_e>(entity_1);
            CHECK(actual_value == shared_component_e1);
        }

        constexpr Shared_component_e shared_component_e2{ .value = 2 };

        entity_manager.set_shared_component(shared_component_e1_key, shared_component_e2);

        {
            Shared_component_e const& actual_value = entity_manager.get_shared_component<Shared_component_e>(shared_component_e1_key);

            CHECK(actual_value == shared_component_e2);
        }

        {
            Shared_component_e const& actual_value = entity_manager.get_shared_component<Shared_component_e>(entity_0);
            CHECK(actual_value == shared_component_e2);
        }

        {
            Shared_component_e const& actual_value = entity_manager.get_shared_component<Shared_component_e>(entity_1);
            CHECK(actual_value == shared_component_e2);
        }


        std::array<Component_type_info, 2> const archetype_ab_component_type_infos =
            make_sorted_component_type_info_array<Component_a, Component_b>();

        Archetype const archetype_abe{archetype_ab_component_type_infos, shared_component_e_type_id, {}};

        Entity const entity_2 = entity_manager.create_entity(archetype_abe, shared_component_e1_key);
        entity_manager.set_component_value(entity_2, Component_a{.value=10});
        entity_manager.set_component_value(entity_2, Component_b{.value=11});

        Entity const entity_3 = entity_manager.create_entity(archetype_abe, shared_component_e1_key);
        entity_manager.set_component_value(entity_3, Component_a{.value=12});
        entity_manager.set_component_value(entity_3, Component_b{.value=13});

        constexpr Shared_component_e shared_component_e3{ .value = 3 };
        constexpr Shared_component_key shared_component_e3_key = 3;

        entity_manager.set_shared_component(shared_component_e3_key, shared_component_e3);

        entity_manager.change_entity_shared_component(entity_2, shared_component_e3_key);

        {
            Shared_component_e const& actual_value = entity_manager.get_shared_component<Shared_component_e>(entity_2);
            CHECK(actual_value == shared_component_e3);
        }

        {
            CHECK(entity_manager.get_component_value<Entity>(entity_2) == entity_2);
            CHECK(entity_manager.get_component_value<Component_a>(entity_2) == Component_a{.value=10});
            CHECK(entity_manager.get_component_value<Component_b>(entity_2) == Component_b{.value=11});
        }

        {
            CHECK(entity_manager.get_component_value<Entity>(entity_3) == entity_3);
            CHECK(entity_manager.get_component_value<Component_a>(entity_3) == Component_a{.value=12});
            CHECK(entity_manager.get_component_value<Component_b>(entity_3) == Component_b{.value=13});
        }
    }

    TEST_CASE("Use entity manager components view", "[entity_manager]")
    {
        Entity_manager entity_manager{};

        std::array<Component_type_info, 3> const archetype_abd_component_type_infos =
            make_sorted_component_type_info_array<Component_a, Component_b, Component_d>();
        
        Archetype const archetype_abd{archetype_abd_component_type_infos, std::nullopt, {}};

        std::array<Component_type_info, 3> const archetype_bcd_component_type_infos =
            make_sorted_component_type_info_array<Component_b, Component_c, Component_d>();

        Archetype const archetype_bcd{archetype_bcd_component_type_infos, std::nullopt, {}};

        Entity const entity_0 = entity_manager.create_entity(archetype_abd);
        entity_manager.set_component_value(entity_0, Component_a{1});
        entity_manager.set_component_value(entity_0, Component_b{2});
        entity_manager.set_component_value(entity_0, Component_d{3});

        Entity const entity_1 = entity_manager.create_entity(archetype_abd);
        entity_manager.set_component_value(entity_1, Component_a{4});
        entity_manager.set_component_value(entity_1, Component_b{5});
        entity_manager.set_component_value(entity_1, Component_d{6});

        Entity const entity_2 = entity_manager.create_entity(archetype_bcd);
        entity_manager.set_component_value(entity_2, Component_b{7});
        entity_manager.set_component_value(entity_2, Component_c{8});
        entity_manager.set_component_value(entity_2, Component_d{9});

        Entity const entity_3 = entity_manager.create_entity(archetype_bcd);
        entity_manager.set_component_value(entity_3, Component_b{10});
        entity_manager.set_component_value(entity_3, Component_c{11});
        entity_manager.set_component_value(entity_3, Component_d{12});

        Entity_manager const& const_entity_manager = entity_manager;

        {
            auto const view = const_entity_manager.get_view<Component_a>();

            REQUIRE(std::distance(view.begin(), view.end()) == 2);

            auto iterator = view.begin();
            CHECK(*iterator++ == Component_a{1});
            CHECK(*iterator++ == Component_a{4});
        }

        {
            auto const view = const_entity_manager.get_view<Component_b>();

            REQUIRE(std::distance(view.begin(), view.end()) == 4);

            auto iterator = view.begin();
            CHECK(*iterator++ == Component_b{2});
            CHECK(*iterator++ == Component_b{5});
            CHECK(*iterator++ == Component_b{7});
            CHECK(*iterator++ == Component_b{10});
        }

        {
            auto const view = const_entity_manager.get_view<Component_b, Component_a, Entity>();

            REQUIRE(std::distance(view.begin(), view.end()) == 2);

            auto iterator = view.begin();
            CHECK(*iterator++ == std::make_tuple(Component_b{2}, Component_a{1}, entity_0));
            CHECK(*iterator++ == std::make_tuple(Component_b{5}, Component_a{4}, entity_1));
        }

        {
            auto const view = const_entity_manager.get_view<Component_d, Component_b, Entity>();

            REQUIRE(std::distance(view.begin(), view.end()) == 4);

            auto iterator = view.begin();
            CHECK(*iterator++ == std::make_tuple(Component_d{3}, Component_b{2}, entity_0));
            CHECK(*iterator++ == std::make_tuple(Component_d{6}, Component_b{5}, entity_1));
            CHECK(*iterator++ == std::make_tuple(Component_d{9}, Component_b{7}, entity_2));
            CHECK(*iterator++ == std::make_tuple(Component_d{12}, Component_b{10}, entity_3));
        }

        {
            auto const view = const_entity_manager.get_view<Component_a, Component_c>();

            CHECK(std::distance(view.begin(), view.end()) == 0);
        }

        {
            auto const view = entity_manager.get_view<Component_b, Component_d>();

            auto const add_b_to_d = [](
                Component_view<Component_b const> const component_b_view,
                Component_view<Component_d> const component_d_view
            ) -> void
            {
                Component_b const component_b = component_d_view.read();     
                
                Component_d component_d = component_b_view.read();
                component_d.value += component_b.value;
                
                component_d_view.write(component_d);
            };

            std::for_each(view.begin(), view.end(), call_with_tuple_arguments(add_b_to_d));

            CHECK(std::distance(view.begin(), view.end()) == 4);

            auto iterator = view.begin();

            CHECK(*iterator++ == std::make_tuple(Component_b{2},Component_d{5}));
            CHECK(*iterator++ == std::make_tuple(Component_b{5},Component_d{11}));
            CHECK(*iterator++ == std::make_tuple(Component_b{7},Component_d{16}));
            CHECK(*iterator++ == std::make_tuple(Component_b{10}, Component_d{22}));
        }
    }

    TEST_CASE("Use entity manager components view grouped by shared components", "[entity_manager]")
    {
        Entity_manager entity_manager{};

        Shared_component_type_ID const shared_component_e_type_id = get_shared_component_type_id<Shared_component_e>();
        Shared_component_type_ID const shared_component_f_type_id = get_shared_component_type_id<Shared_component_f>();

        std::array<Component_type_info, 3> const archetype_abd_component_type_infos =
            make_sorted_component_type_info_array<Component_a, Component_b, Component_d>();
        
        Archetype const archetype_abde{archetype_abd_component_type_infos, shared_component_e_type_id, {}};

        std::array<Component_type_info, 3> const archetype_bcd_component_type_infos =
            make_sorted_component_type_info_array<Component_b, Component_c, Component_d>();

        Archetype const archetype_bcde{archetype_bcd_component_type_infos, shared_component_e_type_id, {}};

        Archetype const archetype_bcdf{archetype_bcd_component_type_infos, shared_component_f_type_id, {}};

        constexpr Shared_component_key key_1{1};
        constexpr Shared_component_key key_2{2};
        constexpr Shared_component_key key_3{3};

        Entity const entity_0 = entity_manager.create_entity(archetype_abde, key_1);
        entity_manager.set_component_value(entity_0, Component_a{1});
        entity_manager.set_component_value(entity_0, Component_b{2});
        entity_manager.set_component_value(entity_0, Component_d{3});

        Entity const entity_1 = entity_manager.create_entity(archetype_abde, key_1);
        entity_manager.set_component_value(entity_1, Component_a{4});
        entity_manager.set_component_value(entity_1, Component_b{5});
        entity_manager.set_component_value(entity_1, Component_d{6});

        Entity const entity_2 = entity_manager.create_entity(archetype_abde, key_2);
        entity_manager.set_component_value(entity_2, Component_a{7});
        entity_manager.set_component_value(entity_2, Component_b{8});
        entity_manager.set_component_value(entity_2, Component_d{9});

        Entity const entity_3 = entity_manager.create_entity(archetype_bcde, key_1);
        entity_manager.set_component_value(entity_3, Component_b{10});
        entity_manager.set_component_value(entity_3, Component_c{11});
        entity_manager.set_component_value(entity_3, Component_d{12});

        Entity const entity_4 = entity_manager.create_entity(archetype_bcde, key_2);
        entity_manager.set_component_value(entity_4, Component_b{13});
        entity_manager.set_component_value(entity_4, Component_c{14});
        entity_manager.set_component_value(entity_4, Component_d{15});

        Entity const entity_5 = entity_manager.create_entity(archetype_bcdf, key_3);
        entity_manager.set_component_value(entity_5, Component_b{16});
        entity_manager.set_component_value(entity_5, Component_c{17});
        entity_manager.set_component_value(entity_5, Component_d{18});

        {
            auto const view = entity_manager.get_view_grouped_by_shared_component<Shared_component_e, Component_a, Component_b>();

            REQUIRE(std::distance(view.begin(), view.end()) == 2);

            auto iterator = view.begin();

            {
                CHECK(iterator.get_chunk_group_key() == key_1);

                auto const underlying_view = *iterator++;

                REQUIRE(std::distance(underlying_view.begin(), underlying_view.end()) == 2);

                auto underlying_iterator = underlying_view.begin();
                CHECK(*underlying_iterator++ == std::make_tuple(Component_a{1}, Component_b{2}));
                CHECK(*underlying_iterator++ == std::make_tuple(Component_a{4}, Component_b{5}));
            }

            {
                CHECK(iterator.get_chunk_group_key() == key_2);

                auto const underlying_view = *iterator++;

                REQUIRE(std::distance(underlying_view.begin(), underlying_view.end()) == 1);

                auto underlying_iterator = underlying_view.begin();
                CHECK(*underlying_iterator++ == std::make_tuple(Component_a{7}, Component_b{8}));
            }
        }

        {
            auto const view = entity_manager.get_view_grouped_by_shared_component<Shared_component_e, Component_b>();

            REQUIRE(std::distance(view.begin(), view.end()) == 4);

            auto iterator = view.begin();

            {
                CHECK(iterator.get_chunk_group_key() == key_1);

                auto const underlying_view = *iterator++;

                REQUIRE(std::distance(underlying_view.begin(), underlying_view.end()) == 2);

                auto underlying_iterator = underlying_view.begin();
                CHECK(*underlying_iterator++ == std::make_tuple(Component_b{2}));
                CHECK(*underlying_iterator++ == std::make_tuple(Component_b{5}));
            }

            {
                CHECK(iterator.get_chunk_group_key() == key_2);

                auto const underlying_view = *iterator++;

                REQUIRE(std::distance(underlying_view.begin(), underlying_view.end()) == 1);

                auto underlying_iterator = underlying_view.begin();
                CHECK(*underlying_iterator++ == std::make_tuple(Component_b{8}));
            }

            {
                CHECK(iterator.get_chunk_group_key() == key_1);

                auto const underlying_view = *iterator++;

                REQUIRE(std::distance(underlying_view.begin(), underlying_view.end()) == 1);

                auto underlying_iterator = underlying_view.begin();
                CHECK(*underlying_iterator++ == std::make_tuple(Component_b{10}));
            }

            {
                CHECK(iterator.get_chunk_group_key() == key_2);

                auto const underlying_view = *iterator++;

                REQUIRE(std::distance(underlying_view.begin(), underlying_view.end()) == 1);

                auto underlying_iterator = underlying_view.begin();
                CHECK(*underlying_iterator++ == std::make_tuple(Component_b{13}));
            }
        }

        {
            auto const view = entity_manager.get_view_grouped_by_shared_component<Shared_component_f, Component_b>();

            REQUIRE(std::distance(view.begin(), view.end()) == 1);

            auto iterator = view.begin();

            {
                CHECK(iterator.get_chunk_group_key() == key_3);

                auto const underlying_view = *iterator++;

                REQUIRE(std::distance(underlying_view.begin(), underlying_view.end()) == 1);

                auto underlying_iterator = underlying_view.begin();
                CHECK(*underlying_iterator++ == std::make_tuple(Component_b{16}));
            }
        }
    }
}
