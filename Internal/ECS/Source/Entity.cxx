export module maia.ecs.entity;

import <cstdint>;
import <functional>;
import <iosfwd>;

namespace Maia::ECS
{
	export struct Entity
	{
		using Integral_type = std::uint32_t;

		Integral_type value{0};
	};

	export bool operator==(Entity lhs, Entity rhs) noexcept;

	export bool operator!=(Entity lhs, Entity rhs) noexcept;

	export std::ostream& operator<<(std::ostream& output_stream, Entity value) noexcept;

	
	export struct Entity_hash
	{
		using argument_type = Maia::ECS::Entity;
		using result_type = std::size_t;

		result_type operator()(argument_type const& entity) const noexcept
		{
			return std::hash<Maia::ECS::Entity::Integral_type>{}(entity.value);
		}
	};
}
