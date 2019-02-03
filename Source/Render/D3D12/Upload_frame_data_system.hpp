#ifndef MAIA_MYTHOLOGY_D3D12_UPLOADFRAMEDATASYSTEM_H_INCLUDED
#define MAIA_MYTHOLOGY_D3D12_UPLOADFRAMEDATASYSTEM_H_INCLUDED

#include "Render_data.hpp"
#include "Renderer.hpp"

namespace Maia::GameEngine
{
	struct Entity_type_id;
	class Entity_manager;
}

namespace Maia::Utilities::glTF
{
	class GlTF;
}

namespace Maia::Mythology::D3D12
{
	class Upload_frame_data_system
	{
	public:

		Upload_frame_data_system(ID3D12Device& device, std::uint8_t pipeline_length);


		void reset(std::uint8_t frame_index);


		std::vector<D3D12_VERTEX_BUFFER_VIEW> upload_instance_data(
			Instance_buffer const& instance_buffer,
			Maia::GameEngine::Entity_manager& entity_manager,
			gsl::span<Maia::GameEngine::Entity_type_id> entity_types_ids
		) const;
		
		void upload_pass_data(Camera camera, ID3D12Resource& pass_buffer, UINT64 pass_buffer_offset) const;


		void execute();

	private:

		winrt::com_ptr<ID3D12CommandQueue> m_command_queue;
		std::vector<winrt::com_ptr<ID3D12CommandAllocator>> m_command_allocators;
		winrt::com_ptr<ID3D12GraphicsCommandList> m_command_list;
		winrt::com_ptr<ID3D12Heap> m_upload_heap;
		winrt::com_ptr<ID3D12Resource> m_upload_buffer;
		UINT64 m_upload_buffer_offset;

	};
}

#endif