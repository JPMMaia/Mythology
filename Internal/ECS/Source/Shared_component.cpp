module maia.ecs.shared_component;

import <cstdint>;
import <mutex>;

namespace Maia::ECS
{
	Shared_component_type_ID Shared_component_type_ID::create_shared_component_id() noexcept
	{
		static std::uint16_t shared_component_type_count = 0;
		static std::mutex mutex;

		{
			std::lock_guard<std::mutex> lock{ mutex };

			return { shared_component_type_count++ };
		}
	}
}
