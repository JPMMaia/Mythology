#ifndef MAIA_MYTHOLOGY_LOADSCENESYSTEM_H_INCLUDED
#define MAIA_MYTHOLOGY_LOADSCENESYSTEM_H_INCLUDED

#include <filesystem>

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
		std::vector<Maia::Utilities::glTF::Scene> scenes;
		std::size_t current_scene_index{ 0 };
	};

	class Load_scene_system
	{
	public:

		explicit Load_scene_system(ID3D12Device& device);


		Scenes_resources load(std::filesystem::path const& gltf_file_path);


	private:

		ID3D12Device& m_device;
		winrt::com_ptr<ID3D12CommandQueue> m_command_queue;
		winrt::com_ptr<ID3D12CommandAllocator> m_command_allocator;
		winrt::com_ptr<ID3D12GraphicsCommandList> m_command_list;
		winrt::com_ptr<ID3D12Heap> m_upload_heap;
		winrt::com_ptr<ID3D12Resource> m_upload_buffer;

	};
}

#endif