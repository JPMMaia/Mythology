#ifndef MAIA_GAMEENGINE_LOCALPOSITION_H_INCLUDED
#define MAIA_GAMEENGINE_LOCALPOSITION_H_INCLUDED

#include <Eigen/Core>

namespace Maia::GameEngine::Components
{
	struct Local_position
	{
		Eigen::Vector3f value{ 0.0f, 0.0f, 0.0f };
	};
}

#endif