#include <iostream>
#include <memory_resource>

#include <d3dx12.h>

#include "Check_hresult.hpp"
#include "D3D12_utilities.hpp"

namespace Maia::Renderer::D3D12
{
	winrt::com_ptr<IDXGIFactory4> create_factory(UINT flags)
	{
		winrt::com_ptr<IDXGIFactory4> factory;
		check_hresult(
			CreateDXGIFactory2(flags, __uuidof(factory), factory.put_void()));
		return factory;
	}
	winrt::com_ptr<IDXGIAdapter3> select_adapter(IDXGIFactory4& factory, bool const select_WARP_adapter)
	{
		if (select_WARP_adapter)
		{
			winrt::com_ptr<IDXGIAdapter3> warp_adapter;
			check_hresult(
				factory.EnumWarpAdapter(__uuidof(warp_adapter), warp_adapter.put_void()));

			return warp_adapter;
		}

		else
		{
			/*winrt::com_ptr<IDXGIFactory6> factory_6;

			if (SUCCEEDED(factory.QueryInterface(__uuidof(IDXGIFactory6), factory_6.put_void())))
			{
				winrt::com_ptr<IDXGIAdapter4> hardware_adapter;
				check_hresult(
					factory_6->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, __uuidof(hardware_adapter), hardware_adapter.put_void()));

				return hardware_adapter;
			}
			else*/
			{
				winrt::com_ptr<IDXGIAdapter1> hardware_adapter;
				check_hresult(
					factory.EnumAdapters1(0, hardware_adapter.put()));

				winrt::com_ptr<IDXGIAdapter3> hardware_adapter_3;
				check_hresult(
					hardware_adapter->QueryInterface(__uuidof(hardware_adapter_3), hardware_adapter_3.put_void()));

				return hardware_adapter_3;
			}
		}
	}

	winrt::com_ptr<ID3D12Device> create_device(IDXGIAdapter& adapter, D3D_FEATURE_LEVEL const minimum_feature_level)
	{
		winrt::com_ptr<ID3D12Device> device;
		check_hresult(
			D3D12CreateDevice(&adapter, minimum_feature_level, __uuidof(device), device.put_void()));

		return device;
	}
	winrt::com_ptr<ID3D12CommandQueue> create_command_queue(ID3D12Device& device, D3D12_COMMAND_LIST_TYPE const type, INT const priority, D3D12_COMMAND_QUEUE_FLAGS const flags, UINT const node_mask)
	{
		D3D12_COMMAND_QUEUE_DESC description;
		description.Type = type;
		description.Priority = priority;
		description.Flags = flags;
		description.NodeMask = node_mask;

		winrt::com_ptr<ID3D12CommandQueue> command_queue;
		device.CreateCommandQueue(&description, __uuidof(command_queue), command_queue.put_void());

		return command_queue;
	}
	winrt::com_ptr<ID3D12CommandAllocator> create_command_allocator(ID3D12Device& device, D3D12_COMMAND_LIST_TYPE const type)
	{
		winrt::com_ptr<ID3D12CommandAllocator> command_allocator;
		check_hresult(
			device.CreateCommandAllocator(type, __uuidof(command_allocator), command_allocator.put_void()));

		return command_allocator;
	}
	std::vector<winrt::com_ptr<ID3D12CommandAllocator>> create_command_allocators(ID3D12Device& device, D3D12_COMMAND_LIST_TYPE type, std::size_t count)
	{
		std::vector<winrt::com_ptr<ID3D12CommandAllocator>> command_allocators;
		command_allocators.reserve(count);

		for (std::size_t i = 0; i < count; ++i)
			command_allocators.emplace_back(create_command_allocator(device, type));

		return command_allocators;
	}
	winrt::com_ptr<ID3D12GraphicsCommandList> create_opened_graphics_command_list(
		ID3D12Device& device, UINT const node_mask, D3D12_COMMAND_LIST_TYPE const type,
		ID3D12CommandAllocator& command_allocator, ID3D12PipelineState* const initial_state
	)
	{
		winrt::com_ptr<ID3D12GraphicsCommandList> command_list;
		check_hresult(
			device.CreateCommandList(node_mask, type, &command_allocator, initial_state, __uuidof(command_list), command_list.put_void()));

		return command_list;
	}
	winrt::com_ptr<ID3D12GraphicsCommandList> create_closed_graphics_command_list(
		ID3D12Device& device, UINT const node_mask, D3D12_COMMAND_LIST_TYPE const type,
		ID3D12CommandAllocator& command_allocator, ID3D12PipelineState* const initial_state
	)
	{
		auto const command_list = create_opened_graphics_command_list(device, node_mask, type, command_allocator, initial_state);
		check_hresult(
			command_list->Close());

		return command_list;
	}
	winrt::com_ptr<ID3D12DescriptorHeap> create_descriptor_heap(ID3D12Device& device, D3D12_DESCRIPTOR_HEAP_TYPE const type, UINT const num_descriptors, D3D12_DESCRIPTOR_HEAP_FLAGS const flags, UINT const node_mask)
	{
		D3D12_DESCRIPTOR_HEAP_DESC description;
		description.Type = type;
		description.NumDescriptors = num_descriptors;
		description.Flags = flags;
		description.NodeMask = node_mask;

		winrt::com_ptr<ID3D12DescriptorHeap> descriptor_heap;
		check_hresult(
			device.CreateDescriptorHeap(&description, __uuidof(descriptor_heap), descriptor_heap.put_void()));

		return descriptor_heap;
	}
	winrt::com_ptr<ID3D12Fence> create_fence(ID3D12Device& device, UINT64 const initial_value, D3D12_FENCE_FLAGS const flags)
	{
		winrt::com_ptr<ID3D12Fence> fence;
		check_hresult(
			device.CreateFence(initial_value, flags, __uuidof(fence), fence.put_void()));

		return fence;
	}

	DXGI_RATIONAL find_refresh_rate(IDXGIAdapter& adapter, UINT const output_index, DXGI_FORMAT const format, std::pair<UINT, UINT> const window_size)
	{
		winrt::com_ptr<IDXGIOutput> output;

		HRESULT const result = adapter.EnumOutputs(output_index, output.put());

		if (result == DXGI_ERROR_NOT_FOUND)
			return {};

		UINT const flags = DXGI_ENUM_MODES_INTERLACED;

		UINT num_modes;
		check_hresult(
			output->GetDisplayModeList(format, flags, &num_modes, nullptr));

		std::vector<DXGI_MODE_DESC> display_modes(num_modes);
		check_hresult(
			output->GetDisplayModeList(format, flags, &num_modes, display_modes.data()));

		for (UINT mode_index = 0; mode_index < num_modes; ++mode_index)
		{
			DXGI_MODE_DESC const& display_mode = display_modes[mode_index];

			if (display_mode.Width == window_size.first && display_mode.Height == window_size.second)
			{
				return display_mode.RefreshRate;
			}
		}

		throw std::runtime_error("Couldn't find a valid refresh rate");
	}
	winrt::com_ptr<IDXGISwapChain3> create_swap_chain(IDXGIFactory2& factory, IUnknown& direct_command_queue, HWND window_handle, DXGI_MODE_DESC1 const& mode, UINT buffer_count, BOOL windowed)
	{
		DXGI_SWAP_CHAIN_DESC1 description{};
		description.Width = mode.Width;
		description.Height = mode.Height;
		description.Format = mode.Format;
		description.Stereo = mode.Stereo;
		description.SampleDesc.Count = 1;
		description.SampleDesc.Quality = 0;
		description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		description.BufferCount = buffer_count;
		description.Scaling = DXGI_SCALING_NONE;
		description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreen_description{};
		fullscreen_description.RefreshRate = mode.RefreshRate;
		fullscreen_description.ScanlineOrdering = mode.ScanlineOrdering;
		fullscreen_description.Scaling = mode.Scaling;
		fullscreen_description.Windowed = windowed;

		winrt::com_ptr<IDXGISwapChain1> swap_chain;
		check_hresult(
			factory.CreateSwapChainForHwnd(&direct_command_queue, window_handle, &description, nullptr, nullptr, swap_chain.put()));

		winrt::com_ptr<IDXGISwapChain3> swap_chain_3;
		swap_chain->QueryInterface(swap_chain_3.put());
		return swap_chain_3;
	}
	winrt::com_ptr<IDXGISwapChain3> create_swap_chain(IDXGIFactory2& factory, IUnknown& direct_command_queue, IUnknown& window, DXGI_FORMAT format, UINT buffer_count)
	{
		DXGI_SWAP_CHAIN_DESC1 description{};
		description.Stereo = true;
		description.Format = format;
		description.SampleDesc.Count = 1;
		description.SampleDesc.Quality = 0;
		description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		description.BufferCount = buffer_count;
		description.Scaling = DXGI_SCALING_NONE;
		description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

		winrt::com_ptr<IDXGISwapChain1> swap_chain;
		check_hresult(
			factory.CreateSwapChainForCoreWindow(&direct_command_queue, &window, &description, nullptr, swap_chain.put()));

		winrt::com_ptr<IDXGISwapChain3> swap_chain_3;
		swap_chain->QueryInterface(swap_chain_3.put());
		return swap_chain_3;
	}
	void create_swap_chain_rtvs(ID3D12Device& device, IDXGISwapChain& swap_chain, DXGI_FORMAT format, D3D12_CPU_DESCRIPTOR_HANDLE destination_descriptor, UINT buffer_count)
	{
		D3D12_RENDER_TARGET_VIEW_DESC description;
		description.Format = format;
		description.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		description.Texture2D.MipSlice = 0;
		description.Texture2D.PlaneSlice = 0;

		UINT const descriptor_handle_increment_size = device.GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		for (UINT buffer_index = 0; buffer_index < buffer_count; ++buffer_index)
		{
			winrt::com_ptr<ID3D12Resource> buffer;
			check_hresult(
				swap_chain.GetBuffer(buffer_index, __uuidof(buffer), buffer.put_void()));

			device.CreateRenderTargetView(buffer.get(), &description, destination_descriptor);

			destination_descriptor.ptr += descriptor_handle_increment_size;
		}
	}

	void resize_swap_chain_buffers_and_recreate_rtvs(IDXGISwapChain3& swap_chain, gsl::span<UINT> create_node_masks, gsl::span<IUnknown*> command_queues, Eigen::Vector2i dimensions, ID3D12Device& device, D3D12_CPU_DESCRIPTOR_HANDLE start_destination_descriptor)
	{
		check_hresult(
			swap_chain.ResizeBuffers1(
				0,
				dimensions(0), dimensions(1),
				DXGI_FORMAT_R8G8B8A8_UNORM,
				{},
				create_node_masks.data(),
				command_queues.data()
			)
		);

		UINT const buffer_count = static_cast<UINT>(create_node_masks.size());
		create_swap_chain_rtvs(
			device,
			swap_chain,
			DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
			start_destination_descriptor,
			buffer_count
		);
	}

	winrt::com_ptr<ID3D12RootSignature> create_root_signature(
		ID3D12Device& device,
		gsl::span<D3D12_ROOT_PARAMETER1 const> root_parameters,
		gsl::span<D3D12_STATIC_SAMPLER_DESC const> static_samplers,
		UINT node_mask,
		D3D12_ROOT_SIGNATURE_FLAGS flags
	)
	{
		D3D12_VERSIONED_ROOT_SIGNATURE_DESC versioned_description;
		versioned_description.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
		versioned_description.Desc_1_1 = [&]() -> D3D12_ROOT_SIGNATURE_DESC1
		{
			D3D12_ROOT_SIGNATURE_DESC1 description{};
			description.NumParameters = static_cast<UINT>(root_parameters.size());
			description.pParameters = root_parameters.data();
			description.NumStaticSamplers = static_cast<UINT>(static_samplers.size());
			description.pStaticSamplers = static_samplers.data();
			description.Flags = flags;
			return description;
		}();

		winrt::com_ptr<ID3DBlob> root_signature_blob;
		winrt::com_ptr<ID3DBlob> error_blob;
		{
			HRESULT const result = D3D12SerializeVersionedRootSignature(&versioned_description, root_signature_blob.put(), error_blob.put());

			if (FAILED(result))
			{
				if (error_blob)
				{
					std::wstring_view error_messages
					{
						reinterpret_cast<wchar_t*>(error_blob->GetBufferPointer()),
						static_cast<std::size_t>(error_blob->GetBufferSize())
					};

					std::cerr << error_messages.data();
				}

				check_hresult(result);
			}
		}

		winrt::com_ptr<ID3D12RootSignature> root_signature;
		check_hresult(
			device.CreateRootSignature(
				node_mask,
				root_signature_blob->GetBufferPointer(), root_signature_blob->GetBufferSize(),
				__uuidof(root_signature), root_signature.put_void()
			)
		);

		return root_signature;
	}

	winrt::com_ptr<ID3D12Heap> create_upload_heap(ID3D12Device& device, UINT64 size_in_bytes)
	{
		assert(size_in_bytes % D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT == 0);

		CD3DX12_HEAP_DESC const description{ size_in_bytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS };

		winrt::com_ptr<ID3D12Heap> upload_heap;
		check_hresult(
			device.CreateHeap(&description, __uuidof(upload_heap), upload_heap.put_void()));
		return upload_heap;
	}
	winrt::com_ptr<ID3D12Heap> create_buffer_heap(ID3D12Device& device, UINT64 size_in_bytes)
	{
		assert(size_in_bytes % D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT == 0);

		CD3DX12_HEAP_PROPERTIES const properties{ D3D12_HEAP_TYPE_DEFAULT };
		CD3DX12_HEAP_DESC const description{ size_in_bytes, properties, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS };

		winrt::com_ptr<ID3D12Heap> upload_heap;
		check_hresult(
			device.CreateHeap(&description, __uuidof(upload_heap), upload_heap.put_void()));
		return upload_heap;
	}
	winrt::com_ptr<ID3D12Resource> create_buffer(ID3D12Device& device, ID3D12Heap& heap, UINT64 heap_offset, UINT64 width, D3D12_RESOURCE_STATES initial_state)
	{
		CD3DX12_RESOURCE_DESC const description = CD3DX12_RESOURCE_DESC::Buffer(width);

		winrt::com_ptr<ID3D12Resource> buffer;
		check_hresult(
			device.CreatePlacedResource(
				&heap,
				heap_offset,
				&description,
				initial_state,
				nullptr,
				__uuidof(buffer),
				buffer.put_void()
			)
		);

		return buffer;
	}

	void wait(
		ID3D12CommandQueue& command_queue,
		ID3D12Fence& fence,
		HANDLE fence_event,
		UINT64 event_value_to_wait,
		DWORD maximum_time_to_wait
	)
	{
		if (fence.GetCompletedValue() < event_value_to_wait)
		{
			check_hresult(
				fence.SetEventOnCompletion(event_value_to_wait, fence_event));

			winrt::check_win32(
				WaitForSingleObject(fence_event, maximum_time_to_wait));
		}
	}

	void signal_and_wait(
		ID3D12CommandQueue& command_queue,
		ID3D12Fence& fence,
		HANDLE fence_event,
		UINT64 event_value_to_signal_and_wait,
		DWORD maximum_time_to_wait
	)
	{
		UINT64 const event_value_to_wait_for = 1;

		check_hresult(
			command_queue.Signal(&fence, event_value_to_signal_and_wait));

		check_hresult(
			fence.SetEventOnCompletion(event_value_to_signal_and_wait, fence_event));

		winrt::check_win32(
			WaitForSingleObject(fence_event, maximum_time_to_wait));
	}
}
