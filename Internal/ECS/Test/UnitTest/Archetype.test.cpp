import maia.ecs.archetype;

import <catch2/catch.hpp>;

namespace Maia::ECS::Test
{
    namespace
    {
        template<typename T>
		struct Component_base{};

		template<typename T>
		bool operator==(Component_base<T> const lhs, Component_base<T> const rhs)
		{
			return true;
		}
		
		template<typename T>
		bool operator!=(Component_base<T> const lhs, Component_base<T> const rhs)
		{
			return !(lhs == rhs);
		}

	    struct Component_a : Component_base<Component_a>{};
		struct Component_b : Component_base<Component_b>{};

        struct Shared_component_a{};
		struct Shared_component_b{};
    }

    TEST_CASE("Use Archetype_info")
    {
        using Archetype_a = 
			Archetype<
				Shared_component_tag<
					Shared_component_a
				>,
				Components_tag<
					Component_a
				>
			>;
		
        Archetype_info const archetype_a_info =
			Archetype_a::get_info();

		CHECK(archetype_a_info.has_component<Component_a>());
		CHECK(!archetype_a_info.has_component<Component_b>());

		CHECK(archetype_a_info.has_shared_component<Shared_component_a>());
		CHECK(!archetype_a_info.has_shared_component<Shared_component_b>());
    }
}
