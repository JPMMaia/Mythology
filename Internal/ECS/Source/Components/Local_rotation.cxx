export module maia.ecs.components.local_rotation;

namespace Maia::ECS::Components
{
	export struct Local_rotation
	{
		float x;
		float y;
		float z;
		float w;
	};

	export inline bool operator==(Local_rotation const& lhs, Local_rotation const& rhs) noexcept
	{
		return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
	}
	export inline bool operator!=(Local_rotation const& lhs, Local_rotation const& rhs) noexcept
	{
		return !(lhs == rhs);
	}
}
