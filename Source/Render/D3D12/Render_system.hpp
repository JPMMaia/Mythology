#ifndef MAIA_MYTHOLOGY_D3D12_RENDERSYSTEM_H_INCLUDED
#define MAIA_MYTHOLOGY_D3D12_RENDERSYSTEM_H_INCLUDED

#include <Maia/Renderer/D3D12/Utilities/Shader.hpp>
#include <Maia/Renderer/D3D12/Utilities/Upload_buffer.hpp>
#include <Maia/Renderer/D3D12/Utilities/Upload_buffer_view.hpp>

#include "Render_data.hpp"
#include "Renderer.hpp"
#include "Components/Mesh_ID.hpp"

#include "Upload_frame_data_system.hpp"

namespace Maia::GameEngine
{
	class Entity_manager;
}

namespace Maia::Utilities::glTF
{
	class GlTF;
}

namespace Maia::Mythology::D3D12
{
	struct Swap_chain
	{
		IDXGISwapChain3& value;
		Eigen::Vector2i bounds;
	};

	class Render_system
	{
	public:

		Render_system(
			ID3D12Device2& device,
			ID3D12CommandQueue& copy_command_queue, 
			ID3D12CommandQueue& direct_command_queue, 
			Swap_chain window,
			std::uint8_t pipeline_length,
			bool vertical_sync
		);


		void render_frame(
			Maia::GameEngine::Entity_manager const& entity_manager,
			Maia::GameEngine::Entity const camera_entity,
			gsl::span<Maia::GameEngine::Entity_type_id const> entity_types_with_mesh,
			gsl::span<Mesh_ID const> entity_type_mesh_indices,
			gsl::span<Maia::Mythology::D3D12::Mesh_view const> mesh_views
		);

		void wait();

		void on_window_resized(Eigen::Vector2i new_size);


	private:

		ID3D12Device& m_device;
		ID3D12CommandQueue& m_copy_command_queue;
		ID3D12CommandQueue& m_direct_command_queue;
		IDXGISwapChain3& m_swap_chain;

		std::uint8_t const m_pipeline_length;
		bool const m_vertical_sync;
		Eigen::Vector2i m_window_size;
		UINT64 m_copy_fence_value;
		winrt::com_ptr<ID3D12Fence> m_copy_fence;

		
		Maia::Mythology::D3D12::Upload_frame_data_system m_upload_frame_data_system;
		Maia::Mythology::D3D12::Renderer m_renderer;
		Maia::Mythology::D3D12::Frames_resources m_frames_resources;

		UINT64 m_submitted_frames;
		winrt::com_ptr<ID3D12Fence> m_fence;
		winrt::handle m_fence_event;

		winrt::com_ptr<ID3D12Resource> m_depth_stencil_buffer;

		
		Maia::Renderer::D3D12::Upload_buffer m_global_upload_buffer;

		winrt::com_ptr<ID3D12Heap> m_pass_heap;
		winrt::com_ptr<ID3D12Resource> m_pass_buffer;
		Maia::Renderer::D3D12::Upload_buffer_view m_pass_data_upload_buffer_view;

		winrt::com_ptr<ID3D12Heap> m_instance_buffers_heap;
		std::vector<Instance_buffer> m_instance_buffer_per_frame;
		Maia::Renderer::D3D12::Upload_buffer m_instances_upload_buffer;

		std::vector<Maia::Renderer::D3D12::Shader> m_shaders;
		std::vector<winrt::com_ptr<ID3D12RootSignature>> m_root_signatures;
		std::vector<winrt::com_ptr<ID3D12PipelineState>> m_pipeline_states;
		D3D12_VIEWPORT m_viewport;
		D3D12_RECT m_scissor_rect;

		std::vector<winrt::com_ptr<ID3D12CommandAllocator>> m_copy_command_allocators;
		winrt::com_ptr<ID3D12GraphicsCommandList4> m_copy_command_list;
		std::vector<winrt::com_ptr<ID3D12CommandAllocator>> m_direct_command_allocators;
		winrt::com_ptr<ID3D12GraphicsCommandList4> m_direct_command_list;
	};
}

#endif
