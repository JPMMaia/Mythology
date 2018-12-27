#ifndef MAIA_MYTHOLOGY_WINDOWSWAPCHAIN_H_INCLUDED
#define MAIA_MYTHOLOGY_WINDOWSWAPCHAIN_H_INCLUDED

#include <winrt/base.h>

#include <Eigen/Core>

#include <dxgi1_6.h>

namespace Maia::Mythology::D3D12
{
	class Window_swap_chain
	{
	public:

		Window_swap_chain(
			IDXGIFactory6& factory, 
			ID3D12CommandQueue& direct_command_queue, 
			IUnknown& window, 
			ID3D12Device& device, 
			D3D12_CPU_DESCRIPTOR_HANDLE destination_descriptor
		);

		void resize(
			ID3D12CommandQueue& direct_command_queue, 
			Eigen::Vector2i window_dimensions, 
			ID3D12Device& device, 
			D3D12_CPU_DESCRIPTOR_HANDLE destination_descriptor
		);

		void present();

		IDXGISwapChain4& get()
		{
			return *m_swap_chain.get();
		}
		IDXGISwapChain4 const& get() const
		{
			return *m_swap_chain.get();
		}

	private:

		static constexpr std::uint8_t buffer_count{ 3 };
		winrt::com_ptr<IDXGISwapChain4> m_swap_chain;

	};
}

#endif
