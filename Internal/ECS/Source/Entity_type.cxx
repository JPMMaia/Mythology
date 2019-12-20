export module maia.ecs.entity_type;

import <cassert>;
import <cstddef>;
import <functional>;
import <iosfwd>;

namespace Maia::ECS
{
	export struct Entity_type_id
	{
		std::size_t value;
	};

	export inline bool operator==(Entity_type_id const lhs, Entity_type_id const rhs) noexcept
	{
		return lhs.value == rhs.value;
	}
	export inline bool operator!=(Entity_type_id const lhs, Entity_type_id const rhs) noexcept
	{
		return !(lhs == rhs);
	}

	export std::ostream& operator<<(std::ostream& output_stream, Entity_type_id const entity_type_id) noexcept;
}

namespace std
{
	export template<> 
	struct hash<Maia::ECS::Entity_type_id>
	{
		using argument_type = Maia::ECS::Entity_type_id;
		using result_type = std::size_t;

		result_type operator()(argument_type const& entity_type) const noexcept
		{
			return std::hash<std::size_t>{}(entity_type.value);
		}
	};
}
