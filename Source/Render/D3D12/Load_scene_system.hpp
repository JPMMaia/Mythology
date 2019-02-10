#ifndef MAIA_MYTHOLOGY_LOADSCENESYSTEM_H_INCLUDED
#define MAIA_MYTHOLOGY_LOADSCENESYSTEM_H_INCLUDED

#include <filesystem>

#include <Maia/GameEngine/Systems/Transform_system.hpp>

#include <Maia/Utilities/glTF/gltf.hpp>

#include "Render_data.hpp"

namespace Maia::Mythology::D3D12
{
	struct Geometry_resources
	{
		winrt::com_ptr<ID3D12Heap> heap;
		Geometry_buffer buffer;
		std::vector<UINT64> offsets;
	};

	struct Scenes_resources
	{
		Geometry_resources geometry_resources;
		std::vector<Mesh_view> mesh_views;
		std::vector<Maia::Utilities::glTF::Node> nodes;
		std::vector<Maia::Utilities::glTF::Scene> scenes;
		std::size_t current_scene_index{ 0 };
	};

	using Static_entity_type = Maia::GameEngine::Entity_type<Maia::GameEngine::Systems::Transform_matrix>;

	class Load_scene_system
	{
	public:

		explicit Load_scene_system(ID3D12Device& device);


		Scenes_resources load(std::filesystem::path const& gltf_file_path);

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

	std::vector<Static_entity_type> create_entities(
		Maia::Utilities::glTF::Scene const& scene,
		gsl::span<Maia::Utilities::glTF::Node const> nodes,
		std::size_t const mesh_count,
		Maia::GameEngine::Entity_manager& entity_manager
	);
	void destroy_entities(
		Maia::GameEngine::Entity_manager& entity_manager
		// TODO entity types
	);
}

#endif