#include <array>

#include <Eigen/Eigen>

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

		winrt::com_ptr<ID3D12Resource> create_buffer(ID3D12Device& device, UINT64 size_in_bytes, D3D12_HEAP_PROPERTIES const& heap_properties, D3D12_RESOURCE_STATES const initial_state, D3D12_CLEAR_VALUE const* const optimized_clear_value = nullptr, D3D12_RESOURCE_FLAGS const flags = {})
		{
			D3D12_RESOURCE_DESC description;
			description.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			description.Alignment = 0;
			description.Width = size_in_bytes;
			description.Height = 1;
			description.DepthOrArraySize = 1;
			description.MipLevels = 1;
			description.Format = DXGI_FORMAT_UNKNOWN;
			description.SampleDesc.Count = 1;
			description.SampleDesc.Quality = 0;
			description.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			description.Flags = flags;
			
			winrt::com_ptr<ID3D12Resource> buffer;
			winrt::check_hresult(
				device.CreateCommittedResource(
					&heap_properties,
					D3D12_HEAP_FLAG_NONE,
					&description,
					initial_state,
					optimized_clear_value,
					__uuidof(buffer),
					buffer.put_void()
				)
			);

			return buffer;
		}

		D3D12_HEAP_PROPERTIES create_upload_heap_properties()
		{
			D3D12_HEAP_PROPERTIES properties;
			properties.Type = D3D12_HEAP_TYPE_UPLOAD;
			properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			properties.CreationNodeMask = 0;
			properties.VisibleNodeMask = 0;

			return properties;
		}

		std::array<Eigen::Vector3f, 3> create_triangle_vertex_data()
		{
			return 
			{
				Eigen::Vector3f(0.0f, 1.0f, 0.0f),
				Eigen::Vector3f(0.5f, -0.5f, 0.0f),
				Eigen::Vector3f(-0.5f, -0.5f, 0.0f),
			};
		}

		template <class T>
		void copy_data(ID3D12Resource& resource, T const* const data, std::size_t const size_in_bytes)
		{
			std::byte* mapped_data;
			winrt::check_hresult(
				resource.Map(0, nullptr, reinterpret_cast<void**>(&mapped_data)));
			
			std::memcpy(mapped_data, data, size_in_bytes);
			
			resource.Unmap(0, nullptr);
		}

		winrt::com_ptr<ID3D12Resource> create_triangle_vertex_buffer(ID3D12Device& device)
		{
			auto const triangle_vertices = create_triangle_vertex_data();
			auto const size_in_bytes = sizeof(decltype(triangle_vertices)::value_type) * triangle_vertices.size();

			auto const upload_heap_properties = create_upload_heap_properties();
			auto const buffer = create_buffer(
				device,
				sizeof(decltype(triangle_vertices)::value_type) * triangle_vertices.size(),
				upload_heap_properties,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				D3D12_RESOURCE_FLAG_NONE
			);

			copy_data(*buffer, triangle_vertices.data(), size_in_bytes);

			return buffer;
		}

		void create_bottom_level_acceleration_structure(ID3D12Device& device, ID3D12GraphicsCommandList& command_list, ID3D12Resource& vertex_buffer)
		{
			D3D12_RAYTRACING_GEOMETRY_DESC description;
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
