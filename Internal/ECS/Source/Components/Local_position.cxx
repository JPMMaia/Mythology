export module maia.ecs.components.local_position;

import <Eigen/Core>;

namespace Maia::ECS::Components
{
	export struct Local_position
	{
		Eigen::Vector3f value{ 0.0f, 0.0f, 0.0f };
	};

	export inline bool operator==(Local_position const& lhs, Local_position const& rhs) noexcept
	{
		return lhs.value == rhs.value;
	}
	export inline bool operator!=(Local_position const& lhs, Local_position const& rhs) noexcept
	{
		return !(lhs == rhs);
	}
}
