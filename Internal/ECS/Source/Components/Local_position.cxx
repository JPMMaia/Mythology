export module maia.ecs.components.local_position;

namespace Maia::ECS::Components
{
	export struct Local_position
	{
		float x;
		float y;
		float z;
	};

	export inline bool operator==(Local_position const& lhs, Local_position const& rhs) noexcept
	{
		return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
	}
	export inline bool operator!=(Local_position const& lhs, Local_position const& rhs) noexcept
	{
		return !(lhs == rhs);
	}
}
