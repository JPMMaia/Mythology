export module maia.ecs.components.local_rotation;

import <Eigen/Geometry>;

namespace Maia::ECS::Components
{
	export struct Local_rotation
	{
		Eigen::Quaternionf value{ 1.0f, 0.0f, 0.0f, 0.0f };
	};

	export inline bool operator==(Local_rotation const& lhs, Local_rotation const& rhs) noexcept
	{
		return lhs.value.isApprox(rhs.value);
	}
	export inline bool operator!=(Local_rotation const& lhs, Local_rotation const& rhs) noexcept
	{
		return !(lhs == rhs);
	}
}
