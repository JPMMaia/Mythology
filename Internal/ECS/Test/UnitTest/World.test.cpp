import maia.ecs.archetype;
import maia.ecs.component;
import maia.ecs.components_chunk;
import maia.ecs.shared_component;
import maia.ecs.world;

import <catch2/catch.hpp>;

import <algorithm>;
import <array>;
import <concepts>;
import <cstring>;
import <ranges>;
import <span>;
import <vector>;

namespace Maia::ECS::Test
{
    namespace
    {
        template<typename T>
		struct Component_base{};

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
            char value;
        };

		struct Component_b : Component_base<Component_b>
        {
            int value = 3;
        };

        struct Component_c : Component_base<Component_c>
        {
            double value;
        };

        struct Component_d : Component_base<Component_d>
        {
            int value;
        };

        struct Shared_component_e
        {
            int value;
        };

        struct Shared_component_f
        {
            float value;
        };

        template<typename Shared_component_t>
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
        }

        bool contains_entity(std::span<Entity const> const entities, Entity const entity) noexcept
        {
            return std::find(entities.begin(), entities.end(), entity) != entities.end();
        }
    }

    TEST_CASE("Use world")
    {
        SECTION("An archetype is an unordered set of component types")
        {
            World world{};

            std::array<Component_type_ID, 2> const archetype_ab_component_type_ids
            {
                get_component_type_id<Component_a>(),
                get_component_type_id<Component_b>()
            };

            Archetype const archetype_ab = world.create_archetype(archetype_ab_component_type_ids);

            CHECK(archetype_ab.has_component(get_component_type_id<Component_a>()) == true);
            CHECK(archetype_ab.has_component(get_component_type_id<Component_b>()) == true);
            CHECK(archetype_ab.has_component(get_component_type_id<Component_c>()) == false);

            {
                Archetype const archetype_ab_clone = world.create_archetype(archetype_ab_component_type_ids);
                CHECK(archetype_ab == archetype_ab_clone);
            }

            std::array<Component_type_ID, 2> const archetype_ba_component_type_ids
            {
                get_component_type_id<Component_b>(),
                get_component_type_id<Component_a>()
            };

            Archetype const archetype_ba = world.create_archetype(archetype_ba_component_type_ids);

            CHECK(archetype_ba.has_component(get_component_type_id<Component_a>()) == true);
            CHECK(archetype_ba.has_component(get_component_type_id<Component_b>()) == true);
            CHECK(archetype_ba.has_component(get_component_type_id<Component_c>()) == false);

            CHECK(archetype_ab == archetype_ba);

            std::array<Component_type_ID, 1> const archetype_a_component_type_ids
            {
                get_component_type_id<Component_a>()
            };

            Archetype const archetype_a = world.create_archetype(archetype_a_component_type_ids);

            CHECK(archetype_ab != archetype_a);
            CHECK(archetype_ba != archetype_a);
        }

        SECTION("Archetypes can have a single shared component type")
        {
            World world{};

            {
                std::array<Component_type_ID, 2> const archetype_ab_component_type_ids
                {
                    get_component_type_id<Component_a>(),
                    get_component_type_id<Component_b>()
                };

                Archetype const archetype_ab = world.create_archetype(archetype_ab_component_type_ids);


                CHECK(archetype_ab.has_shared_component() == false);
                CHECK(archetype_ab.has_shared_component(get_shared_component_type_id<Shared_component_e>()) == false);
                CHECK(archetype_ab.has_shared_component(get_shared_component_type_id<Shared_component_f>()) == false);
            }

            {
                Shared_component_type_ID const shared_component_e_type_id =
                    get_shared_component_type_id<Shared_component_e>();

                std::array<Component_type_ID, 1> const archetype_ae_component_type_ids
                {
                    get_component_type_id<Component_a>()
                };

                Archetype const archetype_ae = world.create_archetype(shared_component_e_type_id, archetype_ae_component_type_ids);


                CHECK(archetype_ae.has_shared_component() == true);
                CHECK(archetype_ae.has_shared_component(get_shared_component_type_id<Shared_component_e>()) == true);
                CHECK(archetype_ae.has_shared_component(get_shared_component_type_id<Shared_component_f>()) == false);
            }
        }

        SECTION("An entity is a tag that identifies components that belong to the same object")
        {
            World world{};

            std::array<Component_type_ID, 2> const archetype_ab_component_type_ids
            {
                get_component_type_id<Component_a>(),
                get_component_type_id<Component_b>()
            };

            Archetype const archetype_ab = world.create_archetype(archetype_ab_component_type_ids);

            Entity const entity_0 = world.create_entity(archetype_ab);

            constexpr Component_a entity_0_a{ .value = 1 };
            world.set_component_value(entity_0, entity_0_a);

            constexpr Component_b entity_0_b{ .value = 2 };
            world.set_component_value(entity_0, entity_0_b);

            Entity const entity_1 = world.create_entity(archetype_ab);

            constexpr Component_a entity_1_a{ .value = 3 };
            world.set_component_value(entity_1, entity_1_a);

            constexpr Component_b entity_1_b{ .value = 4 };
            world.set_component_value(entity_1, entity_1_b);


            CHECK(entity_0 != entity_1);

            {
                Component_a const actual_entity_0_a = world.get_component_value<Component_a>(entity_0);
                CHECK(actual_entity_0_a == entity_0_a);
            }

            {
                Component_b const actual_entity_0_b = world.get_component_value<Component_b>(entity_0);
                CHECK(actual_entity_0_b == entity_0_b);
            }

            {
                Component_a const actual_entity_1_a = world.get_component_value<Component_a>(entity_1);
                CHECK(actual_entity_1_a == entity_1_a);
            }

            {
                Component_b const actual_entity_1_b = world.get_component_value<Component_b>(entity_1);
                CHECK(actual_entity_1_b == entity_1_b);
            }
        }

        SECTION("When an entity is created, its components are default constructed")
        {
            World world{};

            std::array<Component_type_ID, 2> const archetype_b_component_type_ids
            {
                get_component_type_id<Component_b>()
            };

            Archetype const archetype_b = world.create_archetype(archetype_b_component_type_ids);

            Entity const entity_0 = world.create_entity(archetype_b);

            Component_b const initialized_value = world.get_component_value<Component_b>(entity_0);

            Component_b constexpr default_value{};
            CHECK(initialized_value == default_value);
        }

        SECTION("New archetypes can be created by adding or removing components to an entity")
        {
            // TODO
            // TODO use world.get_archetypes()
        }

        SECTION("A shared component is a component that can be shared by multiple entities")
        {
            // TODO how to create a shared component?
            // TODO how to get a shared component?
            // TODO how to update a shared component?
            // TODO how to delete a shared component?
        }

        SECTION("Entity components are grouped by archetype in component chunks")
        {
            World world{};

            std::array<Component_type_ID, 3> const archetype_abd_component_type_ids
            {
                get_component_type_id<Component_a>(),
                get_component_type_id<Component_b>(),
                get_component_type_id<Component_d>(),
            };
            Archetype const archetype_abd = world.create_archetype(archetype_abd_component_type_ids);

            std::array<Component_type_ID, 3> const archetype_bcd_component_type_ids
            {
                get_component_type_id<Component_b>(),
                get_component_type_id<Component_c>(),
                get_component_type_id<Component_d>(),
            };
            Archetype const archetype_bcd = world.create_archetype(archetype_bcd_component_type_ids);

            Entity const entity_0 = world.create_entity(archetype_abd);
            Entity const entity_1 = world.create_entity(archetype_abd);
            Entity const entity_2 = world.create_entity(archetype_bcd);
            Entity const entity_3 = world.create_entity(archetype_bcd);


            CHECK(entity_0 != entity_1);
            CHECK(entity_0 != entity_2);
            CHECK(entity_0 != entity_3);
            CHECK(entity_1 != entity_2);
            CHECK(entity_1 != entity_3);
            CHECK(entity_2 != entity_3);

            {
                std::span<Component_chunk_view const> const component_chunk_views = world.get_component_chunk_views(archetype_abd);
                REQUIRE(component_chunk_views.size() == 1);

                std::span<Entity const> const entities = component_chunk_views[0].get_entities();


                CHECK(entities.size() == 2);
                CHECK(entities[0] == entity_0);
                CHECK(entities[1] == entity_1);
            }

            {
                std::span<Component_chunk_view const> const component_chunk_views = world.get_component_chunk_views(archetype_bcd);
                REQUIRE(component_chunk_views.size() == 1);

                std::span<Entity const> const entities = component_chunk_views[0].get_entities();


                CHECK(entities.size() == 2);
                CHECK(entities[0] == entity_2);
                CHECK(entities[1] == entity_3);
            }
        }

        SECTION("Entity components are grouped by archetype and the value of the shared component in component chunks")
        {
            World world{};

            Shared_component_type_ID const shared_component_e_type_id = get_shared_component_type_id<Shared_component_e>();

            std::array<Component_type_ID, 1> const archetype_a_component_type_ids
            {
                get_component_type_id<Component_a>(),
            };
            Archetype const archetype_ae = world.create_archetype(shared_component_e_type_id, archetype_a_component_type_ids);

            std::array<Component_type_ID, 1> const archetype_b_component_type_ids
            {
                get_component_type_id<Component_a>(),
            };
            Archetype const archetype_be = world.create_archetype(shared_component_e_type_id, archetype_b_component_type_ids);

            Shared_component_e& shared_component_e_0 = world.create_shared_component(Shared_component_e{ .value = 1 });
            Shared_component_e& shared_component_e_1 = world.create_shared_component(Shared_component_e{ .value = 2 });

            Entity const entity_0 = world.create_entity(archetype_ae, shared_component_e_0);
            Entity const entity_1 = world.create_entity(archetype_ae, shared_component_e_1);
            Entity const entity_2 = world.create_entity(archetype_ae, shared_component_e_1);
            Entity const entity_3 = world.create_entity(archetype_ae, shared_component_e_0);
            Entity const entity_4 = world.create_entity(archetype_be, shared_component_e_0);


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
                std::span<Component_chunk_view const> const component_chunk_views = world.get_component_chunk_views(archetype_ae);

                CHECK(component_chunk_views.size() == 2);
                CHECK(contains_shared_component(component_chunk_views, contains_shared_component_0));
                CHECK(contains_shared_component(component_chunk_views, contains_shared_component_1));

                for (Component_chunk_view const component_chunk_view : component_chunk_views)
                {
                    Shared_component_e const& chunk_shared_component = component_chunk_view.get_shared_component<Shared_component_e>();
                    CHECK(chunk_shared_component.value == contains_shared_component_0.value || chunk_shared_component.value == contains_shared_component_1.value);

                    if (chunk_shared_component.value == contains_shared_component_0.value)
                    {
                        std::span<Entity const> const entities = chunk_shared_component.get_entities();

                        CHECK(entities.size() == 2);
                        CHECK(contains_entity(entities, entity_0));
                        CHECK(contains_entity(entities, entity_3));
                    }
                    else if (chunk_shared_component.value == contains_shared_component_1.value)
                    {
                        std::span<Entity const> const entities = chunk_shared_component.get_entities();

                        CHECK(entities.size() == 2);
                        CHECK(contains_entity(entities, entity_1));
                        CHECK(contains_entity(entities, entity_2));
                    }
                }
            }

            {
                std::span<Component_chunk_view const> const component_chunk_views = world.get_component_chunk_views(archetype_be);

                REQUIRE(component_chunk_views.size() == 1);

                Component_chunk_view const component_chunk_view = component_chunk_views[0];

                CHECK(chunk_shared_component.value == contains_shared_component_0.value);

                std::span<Entity const> const entities = chunk_shared_component.get_entities();

                REQUIRE(entities.size() == 1);
                CHECK(entities[0] == entity_4);
            }
        }

        SECTION("Access and change component values from Entity_component_views")
        {
            World world{};

            std::array<Component_type_ID, 1> const archetype_a_component_type_id
            {
                get_component_type_id<Component_a>(),
            };
            Archetype const archetype_a = world.create_archetype(archetype_a_component_type_id);

            Entity const entity_0 = world.create_entity(archetype_a);

            constexpr Component_a original_component_a{ .value = 1 };
            world.set_component_value(entity_0, original_component_a);


            {
                Component_a const actual_component_a = world.get_component_value<Component_a>(entity_0);
                CHECK(actual_component_a == original_component_a);
            }

            {
                {
                    std::span<Component_chunk_view const> const component_chunk_views = world.get_component_chunk_views(archetype_a);
                    REQUIRE(component_chunk_views.size() == 1);
                    REQUIRE(component_chunk_views[0].get_entity_count() == 1);

                    Const_entity_components_view const entity_components_view = component_chunk_views[0].get_entity_components_view(0);

                    Component_a const actual_component_a = entity_components_view.get<Component_a>();
                    CHECK(actual_component_a == original_component_a);
                }

                {
                    std::span<Component_chunk_view> const component_chunk_views = world.get_component_chunk_views(archetype_a);
                    REQUIRE(component_chunk_views.size() == 1);
                    REQUIRE(component_chunk_views[0].get_entity_count() == 1);

                    Entity_components_view const entity_components_view = component_chunk_views[0].get_entity_components_view(0);

                    constexpr Component_a new_component_a{ .value = 2 };
                    entity_components_view.set(new_component_a);

                    Component_a const actual_component_a = entity_components_view.get<Component_a>();
                    CHECK(actual_component_a == new_component_a);
                }
            }
        }
    }
}
