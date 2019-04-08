#ifndef MAIA_MYTHOLOGY_SCENE_H_INCLUDED
#define MAIA_MYTHOLOGY_SCENE_H_INCLUDED

#include <Maia/GameEngine/Entity_manager.hpp>

#include <Renderer/D3D12/Render_data.hpp>

namespace Maia::Mythology
{
	Maia::Mythology::D3D12::Scene_resources load(Maia::GameEngine::Entity_manager& entity_manager, Maia::Mythology::D3D12::Render_resources& render_resources);
}

#endif
