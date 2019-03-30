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
	struct Upload_bundle
	{
		std::uint8_t const frame_index;
		UINT64 offset;
	};

	class Upload_frame_data_system
	{
	public:

		Upload_frame_data_system(ID3D12Device& device, std::uint8_t pipeline_length);


		[[nodiscard]] Upload_bundle reset(std::uint8_t frame_index);


		std::vector<D3D12_VERTEX_BUFFER_VIEW> upload_instance_data(
			Upload_bundle& bundle,
			Instance_buffer const& instance_buffer,
			Maia::GameEngine::Entity_manager const& entity_manager,
			gsl::span<Maia::GameEngine::Entity_type_id const> entity_types_ids
		);
		
		void upload_pass_data(
			Upload_bundle& bundle,
			Pass_data const& pass_data,
			ID3D12Resource& pass_buffer, UINT64 pass_buffer_offset
		);


		ID3D12CommandList& close(Upload_bundle& bundle);

	private:

		std::vector<winrt::com_ptr<ID3D12CommandAllocator>> m_command_allocators;
		winrt::com_ptr<ID3D12GraphicsCommandList> m_command_list;
		winrt::com_ptr<ID3D12Heap> m_upload_heap;
		winrt::com_ptr<ID3D12Resource> m_upload_buffer;

	};
}

#endif