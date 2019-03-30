#ifndef MAIA_MYTHOLOGY_CAMERACOMPONENT_H_INCLUDED
#define MAIA_MYTHOLOGY_CAMERACOMPONENT_H_INCLUDED

#include <Maia/GameEngine/Component.hpp>

#include <Maia/Utilities/glTF/gltf.hpp>

namespace Maia::Mythology
{
	struct Camera_component
	{
		Maia::Utilities::glTF::Camera value;

		static Maia::GameEngine::Component_ID ID()
		{
			return { 32 };
		}
	};
}

#endif