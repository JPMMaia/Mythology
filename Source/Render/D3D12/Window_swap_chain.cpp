#include <Maia/Renderer/D3D12/Utilities/D3D12_utilities.hpp>

#include "Window_swap_chain.hpp"

using namespace Maia::Renderer::D3D12;

namespace Maia::Mythology::D3D12
{
	Window_swap_chain::Window_swap_chain(IDXGIFactory6& factory, ID3D12CommandQueue& direct_command_queue, IUnknown& window, ID3D12Device& device, D3D12_CPU_DESCRIPTOR_HANDLE destination_descriptor) :
		m_swap_chain{ create_swap_chain_and_rtvs(factory, direct_command_queue, window, buffer_count, device, destination_descriptor) }
	{
	}

	void Window_swap_chain::resize(ID3D12CommandQueue& direct_command_queue, Eigen::Vector2i window_dimensions, ID3D12Device& device, D3D12_CPU_DESCRIPTOR_HANDLE destination_descriptor)
	{
		// TODO SIGNAL AND WAIT

		ID3D12CommandQueue& command_queue = direct_command_queue;

		std::array<UINT, buffer_count> create_node_masks;
		std::fill(create_node_masks.begin(), create_node_masks.end(), 1);

		std::array<IUnknown*, buffer_count> command_queues;
		std::fill(command_queues.begin(), command_queues.end(), &command_queue);

		resize_swap_chain_buffers_and_recreate_rtvs(
			*m_swap_chain,
			create_node_masks,
			command_queues,
			window_dimensions,
			device,
			destination_descriptor
		);
	}

	void Window_swap_chain::present()
	{
		winrt::check_hresult(
			m_swap_chain->Present(1, 0));
	}
}