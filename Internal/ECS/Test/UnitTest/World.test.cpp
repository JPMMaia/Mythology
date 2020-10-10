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
            int value;
        };

        struct Component_c : Component_base<Component_c>
        {
            double value;
        };

        struct Component_d : Component_base<Component_d>
        {
            int value;
        };

        struct Shared_component_a
        {
            int value;
        };

        struct Shared_component_b
        {
            float value;
        };
    }

    TEST_CASE("Use world")
    {
        SECTION("Create archetypes")
        {
            World world{};

            std::array<Component_ID, 2> const archetype_0_component_ids
            {
                Component_ID::get<Component_a>(),
                Component_ID::get<Component_b>()
            };

            Archetype const archetype_0 = world.create_archetype(archetype_0_component_ids);

            {
                std::array<Component_ID, 2> const archetype_0_component_ids_reverse
                {
                    Component_ID::get<Component_b>(),
                    Component_ID::get<Component_a>()
                };

                Archetype const archetype_0_clone = world.create_archetype(archetype_0_component_ids_reverse);
                CHECK(archetype_0 == archetype_0_clone);
            }

            CHECK(archetype_0.has_component(Component_ID::get<Component_a>()) == true);
            CHECK(archetype_0.has_component(Component_ID::get<Component_b>()) == true);
            CHECK(archetype_0.has_component(Component_ID::get<Component_c>()) == false);
            CHECK(archetype_0.has_shared_component() == false);


            std::array<Component_ID, 2> const archetype_1_component_ids
            {
                Component_ID::get<Component_a>(),
                Component_ID::get<Component_b>()
            };

            Shared_component_ID const archetype_1_shared_component_id = 
                Shared_component_ID::get<Shared_component_a>();

            Archetype const archetype_1 = world.create_archetype(archetype_1_shared_component_id, archetype_1_component_ids);

            {
                Archetype const archetype_1_clone = world.create_archetype(archetype_1_shared_component_id, archetype_1_component_ids);
                CHECK(archetype_1 == archetype_1_clone);
            }

            CHECK(archetype_1.has_component(Component_ID::get<Component_a>()) == true);
            CHECK(archetype_1.has_component(Component_ID::get<Component_b>()) == true);
            CHECK(archetype_1.has_component(Component_ID::get<Component_c>()) == false);
            CHECK(archetype_1.has_shared_component() == true);
            CHECK(archetype_1.has_shared_component(Shared_component_ID::get<Shared_component_a>()) == true);

            CHECK(archetype_0 != archetype_1);
        }

        SECTION("Create entities")
        {
            World world{};

            std::array<Component_ID, 2> const archetype_0_component_ids
            {
                Component_ID::get<Component_a>(),
                Component_ID::get<Component_b>()
            };

            Archetype const archetype_0 = world.create_archetype(archetype_0_component_ids);

            Entity const entity_0 = world.create_entity(archetype_0);
            Entity const entity_1 = world.create_entity(archetype_0);

            CHECK(entity_0 != entity_1);

            // TODO
        }

        SECTION("Use entities without a shared component")
        {
            World world{};

            // TODO

            std::span<Archetype const> const archetypes = world.get_archetypes();

            // TODO Filter archetypes

            //archetype.has_component(Component_ID::get<Component_c>());
            //archetype.has_shared_component();

            //Component_views const component_views = world.get_component_views(archetypes);

            /*for (Component_view entity_components : component_views)
            {
                Entity const entity = entity_components.get_entity();
                Component_a const component_a = entity_components.get<Component_a>();
                Component_b const component_b = entity_components.get<Component_b>();

                // Call function that takes in component_a and component_b and returns component_c
                Component_c const component_c = compute_value(component_a, component_b);

                entity_components.set<Component_c>(component_c);
            }*/
        }

        SECTION("Use entities with a shared component")
        {
            World world{};

            Shared_component_ID const shared_component_a_id = Shared_component_ID::get<Shared_component_a>();
            Shared_component_a& shared_component_a = world.create_shared_component(Shared_component_a{.value=100});
            
            std::array<Component_ID, 3> const archetype_0_component_ids
            {
                Component_ID::get<Component_a>(),
                Component_ID::get<Component_b>(),
                Component_ID::get<Component_d>(),
            };

            Archetype const archetype_0 = world.create_archetype(shared_component_a_id, archetype_0_component_ids);

            std::array<Component_ID, 3> const archetype_1_component_ids
            {
                Component_ID::get<Component_b>(),
                Component_ID::get<Component_c>(),
                Component_ID::get<Component_d>(),
            };

            Archetype const archetype_1 = world.create_archetype(shared_component_a_id, archetype_1_component_ids);

            Entity const entity_0 = world.create_entity(archetype_0);
            world.set_component_value(entity_0, Component_b{.value=10});

            Entity const entity_1 = world.create_entity(archetype_0);
            world.set_component_value(entity_1, Component_b{.value=20});

            Entity const entity_2 = world.create_entity(archetype_1);
            world.set_component_value(entity_2, Component_b{.value=30});

            Entity const entity_3 = world.create_entity(archetype_1);
            world.set_component_value(entity_3, Component_b{.value=40});


            auto const contains_component_b_and_d_and_shared_component_a = [](Archetype const archetype) -> bool
            {
                return 
                    archetype.has_component(Component_ID::get<Component_b>()) &&
                    archetype.has_component(Component_ID::get<Component_d>()) &&
                    archetype.has_shared_component(Shared_component_ID::get<Shared_component_a>());
            };

            auto const get_component_chunk_views = [&world](Archetype const archetype) -> std::span<Component_chunk_view const>
            {
                return world.get_component_chunk_views(archetype);
            };


            std::vector<Component_chunk_view> const component_b_chunk_views = [&] 
            {
                std::span<Archetype const> const all_archetypes = world.get_archetypes();

                std::vector<Archetype> component_b_archetypes;
                std::copy_if(all_archetypes.begin(), all_archetypes.end(), std::back_inserter(component_b_archetypes), contains_component_b_and_d_and_shared_component_a);

                std::vector<std::span<Component_chunk_view const>> component_b_chunks_per_archetype;
                component_b_chunks_per_archetype.resize(component_b_archetypes.size());
                std::transform(component_b_archetypes.begin(), component_b_archetypes.end(), component_b_chunks_per_archetype.begin(), get_component_chunk_views);

                std::vector<Component_chunk_view> component_b_chunk_views;
                for (std::span<Component_chunk_view const> const views : component_b_chunks_per_archetype)
                {
                    component_b_chunk_views.insert(component_b_chunk_views.end(), views.begin(), views.end());
                }

                return component_b_chunk_views;
            }();

            for (Component_chunk_view const component_chunk_view : component_b_chunk_views)
            {
                Shared_component_a const& shared_component_a = component_chunk_view.get_shared_component<Shared_component_a>();

                for (Entity_components_view const entity_components_view : component_chunk_view)
                {
                    Component_b const component_b = entity_components_view.get<Component_b>();

                    Component_d const component_d
                    {
                        .value = shared_component_a.value + component_b.value
                    };
                    
                    entity_components_view.set(component_d);
                }
            }

            {
                Component_d const component = world.get_component_value<Component_d>(entity_0);
                CHECK(component.value == 110);
            }

            {
                Component_d const component = world.get_component_value<Component_d>(entity_1);
                CHECK(component.value == 120);
            }

            {
                Component_d const component = world.get_component_value<Component_d>(entity_2);
                CHECK(component.value == 130);
            }

            {
                Component_d const component = world.get_component_value<Component_d>(entity_3);
                CHECK(component.value == 140);
            }
        }
    }
}
