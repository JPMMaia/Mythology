import maia.ecs.component;

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

	    struct Component_a : Component_base<Component_a>
        {
            char a;
        };

		struct Component_b : Component_base<Component_b>
        {
            int b;
        };
    }

    TEST_CASE("Use Component_info")
    {
        Component_info const component_info_a0 = create_component_info<Component_a>();

        CHECK(component_info_a0.id == Component_ID::get<Component_a>());
        CHECK(component_info_a0.size == sizeof(Component_a));

        Component_info const component_info_a1 = create_component_info<Component_a>();
        CHECK(component_info_a1.id == component_info_a0.id);
        CHECK(component_info_a1.size == component_info_a0.size);

        Component_info const component_info_b = create_component_info<Component_b>();
        CHECK(component_info_b.id != component_info_a0.id);
        CHECK(component_info_b.size == sizeof(Component_b));
    }
}
