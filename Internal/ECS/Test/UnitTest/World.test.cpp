import maia.ecs.archetype;
import maia.ecs.component;
import maia.ecs.components_chunk;
import maia.ecs.shared_component;
import maia.ecs.world;

import <catch2/catch.hpp>;

import <array>;
import <cstring>;

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

        struct Component_c : Component_base<Component_b>
        {
            double value;
        };

        struct Shared_component_a
        {
            int value;
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
                Archetype const archetype_0_clone = world.create_archetype(archetype_0_component_ids);
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
            //archetype.has_shared_component(Shared_component_ID::<Shared_component_a>());

            Components_view const components_view = world.get_components_views(archetypes);

            for (Components_chunk const components_chunk : components_view)
            {
                
            }

        }

        /*SECTION("Use entities with a shared component")
        {
            //archetype.has_component(Component_ID::get<Component_c>());
            //archetype.has_shared_component();
            //archetype.has_shared_component(Shared_component_ID::<Shared_component_a>());
        }*/
    }
}
