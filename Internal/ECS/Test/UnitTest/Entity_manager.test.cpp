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

    TEST_CASE("New archetypes can be created by adding or removing components to an entity", "[entity_manager]")
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
        constexpr Shared_component_e shared_component_e{ .value = 1 };

        Entity_manager entity_manager{};

        Shared_component_type_ID const shared_component_e_type_id =
            get_shared_component_type_id<Shared_component_e>();

        Archetype const archetype_e{{}, shared_component_e_type_id, {}};

        Entity const entity_0 = entity_manager.create_entity(archetype_e, shared_component_e);

        {
            Shared_component_e const& actual_shared_component =
                entity_manager.get_shared_component_value<Shared_component_e>(entity_0);

            CHECK(shared_component_e == actual_shared_component);
        }

        {
            constexpr Shared_component_e new_shared_component{ .value = 2 };
            entity_manager.set_shared_component_value(entity_0, new_shared_component);

            Shared_component_e const& actual_shared_component =
                entity_manager.get_shared_component_value<Shared_component_e>(entity_0);

            CHECK(new_shared_component == actual_shared_component);
            CHECK(shared_component_e != actual_shared_component);
        }

        {
            {
                Archetype const& original_archetype = entity_manager.get_archetype(entity_0);
                CHECK(original_archetype.has_shared_component(get_shared_component_type_id<Shared_component_e>()));
            }

            entity_manager.remove_shared_component_type<Shared_component_e>(entity_0);

            {
                Archetype const& new_archetype = entity_manager.get_archetype(entity_0);
                CHECK(!new_archetype.has_shared_component(get_shared_component_type_id<Shared_component_e>()));
            }
        }
    }

    /*TEST_CASE("Entity components are grouped by archetype in component chunks", "[entity_manager]")
    {
        Entity_manager entity_manager{};

        std::array<Component_type_ID, 3> const archetype_abd_component_type_ids
        {
            get_component_type_id<Component_a>(),
            get_component_type_id<Component_b>(),
            get_component_type_id<Component_d>(),
        };
        Archetype const archetype_abd{archetype_abd_component_type_ids, {}};

        std::array<Component_type_ID, 3> const archetype_bcd_component_type_ids
        {
            get_component_type_id<Component_b>(),
            get_component_type_id<Component_c>(),
            get_component_type_id<Component_d>(),
        };
        Archetype const archetype_bcd{archetype_bcd_component_type_ids, {}};

        Entity const entity_0 = entity_manager.create_entity(archetype_abd);
        Entity const entity_1 = entity_manager.create_entity(archetype_abd);
        Entity const entity_2 = entity_manager.create_entity(archetype_bcd);
        Entity const entity_3 = entity_manager.create_entity(archetype_bcd);


        CHECK(entity_0 != entity_1);
        CHECK(entity_0 != entity_2);
        CHECK(entity_0 != entity_3);
        CHECK(entity_1 != entity_2);
        CHECK(entity_1 != entity_3);
        CHECK(entity_2 != entity_3);

        {
            std::span<Component_chunk_view const> const component_chunk_views = entity_manager.get_component_chunk_views(archetype_abd);
            REQUIRE(component_chunk_views.size() == 1);

            std::span<Entity const> const entities = component_chunk_views[0].get_entities();


            CHECK(entities.size() == 2);
            CHECK(entities[0] == entity_0);
            CHECK(entities[1] == entity_1);
        }

        {
            std::span<Component_chunk_view const> const component_chunk_views = entity_manager.get_component_chunk_views(archetype_bcd);
            REQUIRE(component_chunk_views.size() == 1);

            std::span<Entity const> const entities = component_chunk_views[0].get_entities();


            CHECK(entities.size() == 2);
            CHECK(entities[0] == entity_2);
            CHECK(entities[1] == entity_3);
        }
    }

    TEST_CASE("Components are grouped by archetype and shared component in chunks", "[entity_manager]")
    {
        Entity_manager entity_manager{};

        Shared_component_type_ID const shared_component_e_type_id = get_shared_component_type_id<Shared_component_e>();

        std::array<Component_type_ID, 1> const archetype_a_component_type_ids
        {
            get_component_type_id<Component_a>(),
        };
        Archetype const archetype_ae{shared_component_e_type_id, archetype_a_component_type_ids, {}};

        std::array<Component_type_ID, 1> const archetype_b_component_type_ids
        {
            get_component_type_id<Component_a>(),
        };
        Archetype const archetype_be{shared_component_e_type_id, archetype_b_component_type_ids, {}};

        Shared_component_e constexpr shared_component_e_0{ .value = 1 };
        Shared_component_e constexpr shared_component_e_1{ .value = 2 };

        Entity const entity_0 = entity_manager.create_entity(archetype_ae, shared_component_e_0);
        Entity const entity_1 = entity_manager.create_entity(archetype_ae, shared_component_e_1);
        Entity const entity_2 = entity_manager.create_entity(archetype_ae, shared_component_e_1);
        Entity const entity_3 = entity_manager.create_entity(archetype_ae, shared_component_e_0);
        Entity const entity_4 = entity_manager.create_entity(archetype_be, shared_component_e_0);


        CHECK(entity_0 != entity_1);
        CHECK(entity_0 != entity_2);
        CHECK(entity_0 != entity_3);
        CHECK(entity_0 != entity_4);
        CHECK(entity_1 != entity_2);
        CHECK(entity_1 != entity_3);
        CHECK(entity_1 != entity_4);
        CHECK(entity_2 != entity_3);
        CHECK(entity_2 != entity_4);
        CHECK(entity_3 != entity_4);

        {
            std::span<Component_chunk_view const> const component_chunk_views = entity_manager.get_component_chunk_views(archetype_ae);

            CHECK(component_chunk_views.size() == 2);

            {
                auto const contains_shared_component_e_0 = [&shared_component_e_0](Component_chunk_view const view) -> bool
                {
                    return view.get_shared_component<Shared_component_e>().value == shared_component_e_0.value;
                };

                auto const shared_component_it = std::find_if(
                    component_chunk_views.begin(),
                    component_chunk_views.end(),
                    contains_shared_component_e_0
                );

                CHECK(shared_component_it != component_chunk_views.end());
            }

            {
                auto const contains_shared_component_e_1 = [&shared_component_e_1](Component_chunk_view const view) -> bool
                {
                    return view.get_shared_component<Shared_component_e>().value == shared_component_e_1.value;
                };

                auto const shared_component_it = std::find_if(
                    component_chunk_views.begin(),
                    component_chunk_views.end(),
                    contains_shared_component_e_1
                );

                CHECK(shared_component_it != component_chunk_views.end());
            }

            //CHECK(contains_shared_component(component_chunk_views, shared_component_e_0));
            //CHECK(contains_shared_component(component_chunk_views, shared_component_e_1));

            for (Component_chunk_view const component_chunk_view : component_chunk_views)
            {
                Shared_component_e const& chunk_shared_component = component_chunk_view.get_shared_component<Shared_component_e>();
                CHECK((chunk_shared_component.value == shared_component_e_0.value || chunk_shared_component.value == shared_component_e_1.value));

                if (chunk_shared_component.value == shared_component_e_0.value)
                {
                    std::span<Entity const> const entities = component_chunk_view.get_entities();

                    CHECK(entities.size() == 2);
                    CHECK(contains(entities, entity_0));
                    CHECK(contains(entities, entity_3));
                }
                else if (chunk_shared_component.value == shared_component_e_1.value)
                {
                    std::span<Entity const> const entities = component_chunk_view.get_entities();

                    CHECK(entities.size() == 2);
                    CHECK(contains(entities, entity_1));
                    CHECK(contains(entities, entity_2));
                }
            }
        }

        {
            std::span<Component_chunk_view const> const component_chunk_views = entity_manager.get_component_chunk_views(archetype_be);

            REQUIRE(component_chunk_views.size() == 1);

            Component_chunk_view const component_chunk_view = component_chunk_views[0];

            Shared_component_e const& chunk_shared_component = component_chunk_view.get_shared_component<Shared_component_e>();
            CHECK(chunk_shared_component.value == shared_component_e_0.value);

            std::span<Entity const> const entities = component_chunk_view.get_entities();

            REQUIRE(entities.size() == 1);
            CHECK(entities[0] == entity_4);
        }
    }

    TEST_CASE("Access and change component values from Entity_component_views", "[entity_manager]")
    {
        Entity_manager entity_manager{};

        std::array<Component_type_ID, 1> const archetype_a_component_type_id
        {
            get_component_type_id<Component_a>(),
        };
        Archetype const archetype_a{archetype_a_component_type_id, {}};

        Entity const entity_0 = entity_manager.create_entity(archetype_a);

        constexpr Component_a original_component_a{ .value = 1 };
        entity_manager.set_component_value(entity_0, original_component_a);


        {
            Component_a const actual_component_a = entity_manager.get_component_value<Component_a>(entity_0);
            CHECK(actual_component_a == original_component_a);
        }

        {
            {
                std::span<Component_chunk_view const> const component_chunk_views = entity_manager.get_component_chunk_views(archetype_a);
                REQUIRE(component_chunk_views.size() == 1);
                REQUIRE(component_chunk_views[0].get_entity_count() == 1);

                Const_entity_components_view const entity_components_view = component_chunk_views[0].get_entity_components_view(0);

                Component_a const actual_component_a = entity_components_view.get<Component_a>();
                CHECK(actual_component_a == original_component_a);
            }

            {
                std::span<Component_chunk_view> const component_chunk_views = entity_manager.get_component_chunk_views(archetype_a);
                REQUIRE(component_chunk_views.size() == 1);
                REQUIRE(component_chunk_views[0].get_entity_count() == 1);

                Entity_components_view const entity_components_view = component_chunk_views[0].get_entity_components_view(0);

                constexpr Component_a new_component_a{ .value = 2 };
                entity_components_view.set(new_component_a);

                Component_a const actual_component_a = entity_components_view.get<Component_a>();
                CHECK(actual_component_a == new_component_a);
            }
        }
    }*/
}
