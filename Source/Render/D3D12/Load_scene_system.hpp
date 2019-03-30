#ifndef MAIA_MYTHOLOGY_LOADSCENESYSTEM_H_INCLUDED
#define MAIA_MYTHOLOGY_LOADSCENESYSTEM_H_INCLUDED

#include <Maia/GameEngine/Systems/Transform_system.hpp>

#include <Maia/Utilities/glTF/gltf.hpp>

#include <Components/Camera_component.hpp>

#include "Render_data.hpp"

namespace Maia::Mythology::D3D12
{
	struct Geometry_resources
	{
		winrt::com_ptr<ID3D12Heap> heap;
		Geometry_buffer buffer;
		std::vector<UINT64> offsets;
	};

	// TODO rename
	struct Scenes_resources
	{
		Geometry_resources geometry_resources;
		std::vector<Mesh_view> mesh_views;
	};


	Maia::Utilities::glTF::Gltf read_gltf(std::filesystem::path const& gltf_file_path);

	class Load_scene_system
	{
	public:

		explicit Load_scene_system(ID3D12Device& device);


		Scenes_resources load(Maia::Utilities::glTF::Gltf const& gltf);

		void wait();

	private:

		ID3D12Device& m_device;
		winrt::com_ptr<ID3D12CommandQueue> m_command_queue;
		winrt::com_ptr<ID3D12CommandAllocator> m_command_allocator;
		winrt::com_ptr<ID3D12GraphicsCommandList> m_command_list;
		winrt::com_ptr<ID3D12Heap> m_upload_heap;
		winrt::com_ptr<ID3D12Resource> m_upload_buffer;
		UINT64 m_fence_value;
		winrt::com_ptr<ID3D12Fence> m_fence;
		winrt::handle m_fence_event;

	};

	struct Scene_entities
	{
		std::vector<Maia::GameEngine::Entity_type_id> mesh;
		std::vector<Maia::GameEngine::Entity> cameras;
	};

	Scene_entities create_entities(
		Maia::Utilities::glTF::Gltf const& gltf,
		Maia::Utilities::glTF::Scene const& scene,
		Maia::GameEngine::Entity_manager& entity_manager
	);
	void destroy_entities(
		Maia::GameEngine::Entity_manager& entity_manager
		// TODO entity types
	);
}

#endif