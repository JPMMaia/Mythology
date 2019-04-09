#ifndef MAIA_GAMEENGINE_LOCALROTATION_H_INCLUDED
#define MAIA_GAMEENGINE_LOCALROTATION_H_INCLUDED

#include <Eigen/Geometry>

namespace Maia::GameEngine::Components
{
	struct Local_rotation
	{
		Eigen::Quaternionf value{ 1.0f, 0.0f, 0.0f, 0.0f };
	};
}

#endif