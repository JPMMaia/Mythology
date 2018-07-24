#include <Maia/Renderer/D3D12/Utilities/D3D12_utilities.h>

#include <D3D12_renderer.h>

using namespace Maia::Renderer::D3D12;

namespace Mythology
{

	namespace
	{
		winrt::com_ptr<ID3D12Device> create_dxr_device(IDXGIAdapter& adapter, D3D_FEATURE_LEVEL const minimum_feature_level)
		{
			winrt::check_hresult(
				D3D12EnableExperimentalFeatures(1, &D3D12ExperimentalShaderModels, nullptr, nullptr));

			return create_device(adapter, minimum_feature_level);
		}
	}
	D3D12_renderer::D3D12_renderer() :
		m_pipeline_length{ 3 },
		m_factory{ create_factory({}) },
		m_adapter{ select_adapter(*m_factory, true) },
		m_device{ create_dxr_device(*m_adapter, D3D_FEATURE_LEVEL_12_0) },
		m_direct_command_queue{ create_command_queue(*m_device, D3D12_COMMAND_LIST_TYPE_DIRECT, 0, D3D12_COMMAND_QUEUE_FLAG_NONE, 0) },
		m_command_allocator{ create_command_allocator(*m_device, D3D12_COMMAND_LIST_TYPE_DIRECT) },
		m_command_list{ create_closed_graphics_command_list(*m_device, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, *m_command_allocator, nullptr) },
		m_rtv_descriptor_heap{ create_descriptor_heap(*m_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, static_cast<UINT>(m_pipeline_length), D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0) },
		m_fence_value{ 0 },
		m_fence{ create_fence(*m_device, m_fence_value, D3D12_FENCE_FLAG_NONE) },
		m_fence_event{ ::CreateEvent(nullptr, false, false, nullptr) },
		m_swap_chain{}
	{
	}

	namespace
	{
		winrt::com_ptr<IDXGISwapChain1> create_swap_chain(IDXGIFactory2& factory, IUnknown& direct_command_queue, IUnknown& window, UINT buffer_count)
		{
			DXGI_SWAP_CHAIN_DESC1 description;
			description.Width = 0;
			description.Height = 0;
			description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			description.Stereo = true;
			description.SampleDesc.Count = 1;
			description.SampleDesc.Quality = 0;
			description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			description.BufferCount = buffer_count;
			description.Scaling = DXGI_SCALING_NONE;
			description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			description.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
			description.Flags = {};

			winrt::com_ptr<IDXGISwapChain1> swap_chain;
			winrt::check_hresult(
				factory.CreateSwapChainForCoreWindow(
					&direct_command_queue,
					&window,
					&description,
					nullptr,
					swap_chain.put()
				)
			);

			return swap_chain;
		}
	}

	void D3D12_renderer::window(IUnknown& window)
	{
		m_swap_chain = create_swap_chain(
			*m_factory, 
			*m_direct_command_queue, 
			window, 
			static_cast<UINT>(m_pipeline_length)
		);
	}
}
