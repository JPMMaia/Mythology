#ifndef MAIA_MYTHOLOGY_D3D12_RENDERSYSTEM_H_INCLUDED
#define MAIA_MYTHOLOGY_D3D12_RENDERSYSTEM_H_INCLUDED

#include "Render_data.hpp"
#include "Renderer.hpp"
#include "Window_swap_chain.hpp"

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
	struct Window
	{
		::IUnknown& value;
		Eigen::Vector2i bounds;
	};

	class Render_system
	{
	public:

		explicit Render_system(Window const& window);


		void render_frame(Maia::GameEngine::Entity_manager const& entity_manager);

		void wait();

		void on_window_resized(Eigen::Vector2i new_size);


		ID3D12Device5& d3d12_device() const { return *m_render_resources.device; }

	private:

		winrt::com_ptr<IDXGIFactory6> m_factory;
		winrt::com_ptr<IDXGIAdapter4> m_adapter;

		Maia::Mythology::D3D12::Render_resources m_render_resources;
		winrt::com_ptr<ID3D12CommandQueue> m_copy_command_queue;
		winrt::com_ptr<ID3D12CommandQueue> m_direct_command_queue;

		Maia::Mythology::D3D12::Upload_frame_data_system m_upload_frame_data_system;
		Maia::Mythology::D3D12::Renderer m_renderer;
		Maia::Mythology::D3D12::Window_swap_chain m_window_swap_chain;
		Maia::Mythology::D3D12::Frames_resources m_frames_resources;

		std::uint8_t const m_pipeline_length;
		UINT64 m_fence_value;
		winrt::com_ptr<ID3D12Fence> m_fence;
		winrt::handle m_fence_event;
		std::size_t m_submitted_frames;

		winrt::com_ptr<ID3D12Heap> m_pass_heap;
		winrt::com_ptr<ID3D12Resource> m_pass_buffer;

		winrt::com_ptr<ID3D12Heap> m_instance_buffers_heap;
		std::vector<Instance_buffer> m_instance_buffer_per_frame;
	};
}

#endif
