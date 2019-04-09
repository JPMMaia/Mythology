#include "Component.hpp"

#include <mutex>

namespace Maia::GameEngine
{
	Component_ID Component_ID::create_component_id()
	{
		static std::uint16_t component_type_count = 0;
		static std::mutex mutex;

		{
			std::lock_guard<std::mutex> lock{ mutex };

			return { component_type_count++ };
		}
	}
}
