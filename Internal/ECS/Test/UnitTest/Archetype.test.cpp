import maia.ecs.archetype;

import <catch2/catch.hpp>;

import <type_traits>;

namespace Maia::ECS::Test
{
	struct Component_a{};

	using Archetype_a = 
		Archetype<
			Components_tag<
				Component_a
			>
		>;

	static_assert(Archetype_a::has_shared_component() == false);
	static_assert(Archetype_a::get_component_count() == 1);
	//static_assert(std::is_same_v<Archetype_a::Nth_component<0>, Component_a>);


	struct Component_b{};

	using Archetype_b = 
		Archetype<
			Components_tag<
				Component_a,
				Component_b
			>
		>;

	static_assert(Archetype_b::has_shared_component() == false);
	static_assert(Archetype_b::get_component_count() == 2);
	//static_assert(std::is_same_v<Archetype_b::Nth_component<0>, Component_a>);
	//static_assert(std::is_same_v<Archetype_b::Nth_component<1>, Component_b>);


	struct Shared_component_a{};

	using Archetype_c = 
		Archetype<
			Shared_component_tag<
				Shared_component_a
			>,
			Components_tag<
				Component_a
			>
		>;

	static_assert(Archetype_c::has_shared_component() == true);
	static_assert(std::is_same_v<Archetype_c::Shared_component, Shared_component_a>);
	static_assert(Archetype_c::get_component_count() == 1);
	//static_assert(std::is_same_v<Archetype_c::Nth_component<0>, Component_a>);
}
