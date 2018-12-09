#include <array>
#include <iostream>

#include <Eigen/Eigen>
#include <gsl/gsl>

#include <Maia/Renderer/D3D12/Utilities/D3D12_utilities.hpp>
#include <Maia/Renderer/D3D12/Utilities/Mapped_memory.hpp>

#include "Renderer.hpp"

using namespace Maia::Renderer::D3D12;

namespace Mythology::D3D12
{
	namespace
	{
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
				winrt::check_hresult(
					swap_chain.GetBuffer(buffer_index, __uuidof(buffer), buffer.put_void()));

				device.CreateRenderTargetView(buffer.get(), &description, destination_descriptor);

				destination_descriptor.ptr += descriptor_handle_increment_size;
			}
		}

		winrt::com_ptr<IDXGISwapChain4> create_rtv_swap_chain(IDXGIFactory6& factory, IUnknown& direct_command_queue, IUnknown& window, UINT buffer_count, ID3D12Device& device, D3D12_CPU_DESCRIPTOR_HANDLE destination_descriptor)
		{
			winrt::com_ptr<IDXGISwapChain4> swap_chain =
				create_swap_chain(factory, direct_command_queue, window, DXGI_FORMAT_R8G8B8A8_UNORM, buffer_count);

			create_swap_chain_rtvs(device, *swap_chain, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, destination_descriptor, buffer_count);

			return swap_chain;
		}

		winrt::com_ptr<ID3D12RootSignature> create_root_signature(ID3D12Device5& device)
		{
			D3D12_VERSIONED_ROOT_SIGNATURE_DESC versioned_description;
			versioned_description.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
			versioned_description.Desc_1_1 = []() -> D3D12_ROOT_SIGNATURE_DESC1
			{
				D3D12_ROOT_SIGNATURE_DESC1 description{};
				description.NumParameters = 0;
				description.pParameters = nullptr;
				description.NumStaticSamplers = 0;
				description.pStaticSamplers = nullptr;
				description.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
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

					winrt::check_hresult(result);
				}
			}

			winrt::com_ptr<ID3D12RootSignature> root_signature;
			winrt::check_hresult(
				device.CreateRootSignature(
					0,
					root_signature_blob->GetBufferPointer(), root_signature_blob->GetBufferSize(),
					__uuidof(root_signature), root_signature.put_void()
				)
			);

			return root_signature;
		}

		winrt::com_ptr<ID3D12PipelineState> create_color_pass_pipeline_state(ID3D12Device5& device, ID3D12RootSignature& root_signature, D3D12_SHADER_BYTECODE vertex_shader, D3D12_SHADER_BYTECODE pixel_shader)
		{
			D3D12_GRAPHICS_PIPELINE_STATE_DESC description{};
			description.pRootSignature = &root_signature;
			description.VS = vertex_shader;
			description.PS = pixel_shader;
			description.DS;
			description.HS;
			description.GS;
			description.StreamOutput;
			description.BlendState = []() -> D3D12_BLEND_DESC
			{
				D3D12_BLEND_DESC blend_state{};
				blend_state.AlphaToCoverageEnable = false;
				blend_state.IndependentBlendEnable = false;
				blend_state.RenderTarget[0] = []() -> D3D12_RENDER_TARGET_BLEND_DESC
				{
					D3D12_RENDER_TARGET_BLEND_DESC render_target_blend_desc{};
					render_target_blend_desc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
					return render_target_blend_desc;
				}();
				return blend_state;
			}();
			description.SampleMask = 0xFFFFFFFF;
			description.RasterizerState = []() -> D3D12_RASTERIZER_DESC
			{
				D3D12_RASTERIZER_DESC rasterizer_state{};
				rasterizer_state.FillMode = D3D12_FILL_MODE_SOLID;
				rasterizer_state.CullMode = D3D12_CULL_MODE_BACK;
				rasterizer_state.FrontCounterClockwise = true;
				rasterizer_state.DepthBias = false;
				rasterizer_state.SlopeScaledDepthBias = 0;
				rasterizer_state.DepthBiasClamp = 0;
				rasterizer_state.DepthClipEnable = true;
				rasterizer_state.MultisampleEnable = false;
				rasterizer_state.AntialiasedLineEnable = false;
				rasterizer_state.ForcedSampleCount = 0;
				rasterizer_state.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
				return rasterizer_state;
			}();
			description.DepthStencilState;

			std::array<D3D12_INPUT_ELEMENT_DESC, 2> input_layout_elements
			{
				D3D12_INPUT_ELEMENT_DESC
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};
			description.InputLayout = { input_layout_elements.data(), static_cast<UINT>(input_layout_elements.size()) };

			description.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
			description.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			description.NumRenderTargets = 1;
			description.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			description.DSVFormat;
			description.SampleDesc.Count = 1;
			description.SampleDesc.Quality = 0;
			description.NodeMask;
			description.CachedPSO;
			description.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

			winrt::com_ptr<ID3D12PipelineState> pipeline_state;
			winrt::check_hresult(
				device.CreateGraphicsPipelineState(&description, __uuidof(pipeline_state), pipeline_state.put_void()));
			return pipeline_state;
		}

		winrt::com_ptr<ID3D12Resource> create_buffer(
			ID3D12Device& device,
			ID3D12Heap& heap,
			UINT64 heap_offset,
			UINT64 width,
			D3D12_RESOURCE_STATES initial_state = D3D12_RESOURCE_STATE_COPY_DEST
		)
		{
			D3D12_RESOURCE_DESC description{};
			description.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			description.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
			description.Width = width;
			description.Height = 1;
			description.DepthOrArraySize = 1;
			description.MipLevels = 1;
			description.Format = DXGI_FORMAT_UNKNOWN;
			description.SampleDesc.Count = 1;
			description.SampleDesc.Quality = 0;
			description.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			description.Flags = D3D12_RESOURCE_FLAG_NONE;

			winrt::com_ptr<ID3D12Resource> buffer;
			winrt::check_hresult(
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

		template <class T>
		void upload_buffer_data(
			ID3D12GraphicsCommandList& command_list,
			ID3D12Resource& destination_buffer, UINT64 destination_buffer_offset,
			ID3D12Resource& upload_buffer, UINT64 upload_buffer_offset,
			gsl::span<T> data
		)
		{
			using namespace Maia::Renderer::D3D12;

			Mapped_memory mapped_memory{ upload_buffer, 0, {} };
			mapped_memory.write(data.data(), data.size_bytes(), upload_buffer_offset);

			command_list.CopyBufferRegion(
				&destination_buffer,
				destination_buffer_offset,
				&upload_buffer,
				upload_buffer_offset,
				data.size_bytes()
			);
		}

		Triangle create_triangle(
			ID3D12Device& device,
			ID3D12Heap& heap, UINT64 heap_offset,
			ID3D12GraphicsCommandList& command_list,
			ID3D12Resource& upload_buffer,
			UINT64 upload_buffer_offset
		)
		{
			struct Vertex
			{
				Eigen::Vector4f positionH;
				Eigen::Vector4f color;
			};

			std::array<Vertex, 3> vertices
			{
				Vertex
				{ { -1.0f, -1.0f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
				{ { 1.0f, -1.0f, 0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
				{ { 0.0f, 1.0f, 0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
			};

			std::array<std::uint16_t, 3> indices
			{
				0, 1, 2
			};

			Triangle triangle;

			UINT64 const vertex_buffer_width = vertices.size() * sizeof(decltype(vertices)::value_type);
			triangle.vertex_buffer =
				create_buffer(
					device,
					heap,
					heap_offset,
					vertex_buffer_width,
					D3D12_RESOURCE_STATE_COPY_DEST
				);

			triangle.index_buffer =
				create_buffer(
					device,
					heap,
					heap_offset + D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
					indices.size() * sizeof(decltype(indices)::value_type),
					D3D12_RESOURCE_STATE_COPY_DEST
				);

			upload_buffer_data<Vertex>(command_list, *triangle.vertex_buffer, 0, upload_buffer, upload_buffer_offset, vertices);
			upload_buffer_data<std::uint16_t>(command_list, *triangle.index_buffer, 0, upload_buffer, upload_buffer_offset + vertex_buffer_width, indices);

			return triangle;
		}

		void submit_resource_barriers(ID3D12GraphicsCommandList& command_list, Triangle const& triangle)
		{
			std::array<D3D12_RESOURCE_BARRIER, 2> resource_barriers;
			{
				D3D12_RESOURCE_BARRIER& resource_barrier = resource_barriers[0];
				resource_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				resource_barrier.Transition.pResource = triangle.vertex_buffer.get();
				resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
				resource_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
				resource_barrier.Transition.Subresource = 0;
				resource_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			}
			{
				D3D12_RESOURCE_BARRIER& resource_barrier = resource_barriers[1];
				resource_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				resource_barrier.Transition.pResource = triangle.index_buffer.get();
				resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
				resource_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_INDEX_BUFFER;
				resource_barrier.Transition.Subresource = 0;
				resource_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			}
			command_list.ResourceBarrier(static_cast<UINT>(resource_barriers.size()), resource_barriers.data());
		}

		winrt::com_ptr<ID3D12Heap> create_upload_heap(ID3D12Device& device, UINT64 size_in_bytes)
		{
			assert(size_in_bytes % D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT == 0);

			D3D12_HEAP_DESC description;
			description.SizeInBytes = size_in_bytes;
			description.Properties = []() -> D3D12_HEAP_PROPERTIES
			{
				D3D12_HEAP_PROPERTIES properties;
				properties.Type = D3D12_HEAP_TYPE_UPLOAD;
				properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
				properties.CreationNodeMask = 1;
				properties.VisibleNodeMask = 1;
				return properties;
			}();
			description.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
			description.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
		
			winrt::com_ptr<ID3D12Heap> upload_heap;
			winrt::check_hresult(
				device.CreateHeap(&description, __uuidof(upload_heap), upload_heap.put_void()));
			return upload_heap;
		}

		winrt::com_ptr<ID3D12Heap> create_buffer_heap(ID3D12Device& device, UINT64 size_in_bytes)
		{
			assert(size_in_bytes % D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT == 0);

			D3D12_HEAP_DESC description;
			description.SizeInBytes = size_in_bytes;
			description.Properties = []() -> D3D12_HEAP_PROPERTIES
			{
				D3D12_HEAP_PROPERTIES properties;
				properties.Type = D3D12_HEAP_TYPE_DEFAULT;
				properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
				properties.CreationNodeMask = 1;
				properties.VisibleNodeMask = 1;
				return properties;
			}();
			description.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
			description.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;

			winrt::com_ptr<ID3D12Heap> upload_heap;
			winrt::check_hresult(
				device.CreateHeap(&description, __uuidof(upload_heap), upload_heap.put_void()));
			return upload_heap;
		}
	}

	Renderer::Renderer(IUnknown& window, Eigen::Vector2i window_dimensions) :
		m_factory{ create_factory({}) },
		m_adapter{ select_adapter(*m_factory, false) },
		m_device{ create_device(*m_adapter, D3D_FEATURE_LEVEL_11_0) },
		m_direct_command_queue{ create_command_queue(*m_device, D3D12_COMMAND_LIST_TYPE_DIRECT, 0, D3D12_COMMAND_QUEUE_FLAG_NONE, 0) },
		m_command_allocators{ create_command_allocators(*m_device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pipeline_length) },
		m_command_list{ create_opened_graphics_command_list(*m_device, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, *m_command_allocators[0], nullptr) },
		m_rtv_descriptor_heap{ create_descriptor_heap(*m_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, static_cast<UINT>(m_pipeline_length), D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0) },
		m_fence_value{ 0 },
		m_fence{ create_fence(*m_device, m_fence_value, D3D12_FENCE_FLAG_NONE) },
		m_fence_event{ ::CreateEvent(nullptr, false, false, nullptr) },
		m_swap_chain{ create_rtv_swap_chain(*m_factory, *m_direct_command_queue, window, m_swap_chain_buffer_count, *m_device, m_rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart()) },
		m_viewport{ 0.0f, 0.0f, static_cast<FLOAT>(window_dimensions(0)), static_cast<FLOAT>(window_dimensions(1)), D3D12_MIN_DEPTH, D3D12_MAX_DEPTH },
		m_scissor_rect{ 0, 0, window_dimensions(0), window_dimensions(1) },
		m_submitted_frames{ m_pipeline_length },
		m_root_signature{ create_root_signature(*m_device) },
		m_color_vertex_shader{ "Resources/Shaders/Color_vertex_shader.csv" },
		m_color_pixel_shader{ "Resources/Shaders/Color_pixel_shader.csv" },
		m_color_pass_pipeline_state{ create_color_pass_pipeline_state(*m_device, *m_root_signature, m_color_vertex_shader.bytecode(), m_color_pixel_shader.bytecode()) },
		m_upload_heap{ create_upload_heap(*m_device, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT) },
		m_upload_buffer{ create_buffer(*m_device, *m_upload_heap, 0, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, D3D12_RESOURCE_STATE_GENERIC_READ) },
		m_buffers_heap{ create_buffer_heap(*m_device, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT * 2) },
		m_triangle{ create_triangle(*m_device, *m_buffers_heap, 0, *m_command_list, *m_upload_buffer, 0) }
	{
		submit_resource_barriers(*m_command_list, m_triangle);
		
		winrt::check_hresult(
			m_command_list->Close());


		{
			std::array<ID3D12CommandList*, 1> command_lists_to_execute
			{
				m_command_list.get()
			};

			m_direct_command_queue->ExecuteCommandLists(
				static_cast<UINT>(command_lists_to_execute.size()), command_lists_to_execute.data()
			);
		}

		{
			UINT64 const event_value_to_wait_for = 1;

			winrt::check_hresult(
				m_direct_command_queue->Signal(m_fence.get(), event_value_to_wait_for));

			winrt::check_hresult(
				m_fence->SetEventOnCompletion(event_value_to_wait_for, m_fence_event.get()));

			winrt::check_win32(
				WaitForSingleObject(m_fence_event.get(), INFINITE));
		}
	}

	void Renderer::resize_window(Eigen::Vector2i window_dimensions)
	{
		// TODO refactor
		{
			UINT64 const event_value_to_wait_for = m_submitted_frames + m_pipeline_length;

			winrt::check_hresult(
				m_direct_command_queue->Signal(m_fence.get(), event_value_to_wait_for));

			winrt::check_hresult(
				m_fence->SetEventOnCompletion(event_value_to_wait_for, m_fence_event.get()));

			winrt::check_win32(
				WaitForSingleObject(m_fence_event.get(), INFINITE));
		}

		std::array<UINT, m_swap_chain_buffer_count> create_node_masks;
		std::fill(create_node_masks.begin(), create_node_masks.end(), 1);

		std::array<IUnknown*, m_swap_chain_buffer_count> command_queues;
		std::fill(command_queues.begin(), command_queues.end(), m_direct_command_queue.get());

		winrt::check_hresult(
			m_swap_chain->ResizeBuffers1(
				0,
				window_dimensions(0), window_dimensions(1),
				DXGI_FORMAT_R8G8B8A8_UNORM,
				{},
				create_node_masks.data(),
				command_queues.data()
			)
		);

		create_swap_chain_rtvs(
			*m_device, 
			*m_swap_chain, 
			DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 
			m_rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), 
			m_swap_chain_buffer_count
		);

		m_viewport.Width = static_cast<FLOAT>(window_dimensions(0));
		m_viewport.Height = static_cast<FLOAT>(window_dimensions(1));
		m_scissor_rect.right = window_dimensions(0);
		m_scissor_rect.bottom = window_dimensions(1);
		
	}

	void Renderer::render()
	{
		{
			UINT64 const event_value_to_wait_for = m_submitted_frames - m_pipeline_length;
			if (m_fence->GetCompletedValue() < event_value_to_wait_for)
			{
				winrt::check_hresult(
					m_fence->SetEventOnCompletion(event_value_to_wait_for, m_fence_event.get()));

				winrt::check_win32(
					WaitForSingleObject(m_fence_event.get(), INFINITE));

				// TODO return to do other cpu work instead of waiting
			}
		}

		{
			UINT const back_buffer_index = m_swap_chain->GetCurrentBackBufferIndex();

			winrt::com_ptr<ID3D12Resource> back_buffer;
			winrt::check_hresult(
				m_swap_chain->GetBuffer(back_buffer_index, __uuidof(back_buffer), back_buffer.put_void()));

			std::uint8_t current_frame_index{ m_submitted_frames % m_pipeline_length };
			ID3D12CommandAllocator& command_allocator = *m_command_allocators[current_frame_index];
			ID3D12GraphicsCommandList& command_list = *m_command_list;

			winrt::check_hresult(
				command_allocator.Reset());

			winrt::check_hresult(
				command_list.Reset(&command_allocator, m_color_pass_pipeline_state.get()));

			command_list.RSSetViewports(1, &m_viewport);
			command_list.RSSetScissorRects(1, &m_scissor_rect);

			{
				D3D12_RESOURCE_BARRIER resource_barrier;
				resource_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				resource_barrier.Transition.pResource = back_buffer.get();
				resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
				resource_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
				resource_barrier.Transition.Subresource = 0;
				resource_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				command_list.ResourceBarrier(1, &resource_barrier);
			}

			{
				UINT const descriptor_handle_increment_size =
					m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

				D3D12_CPU_DESCRIPTOR_HANDLE const render_target_descriptor
				{
					m_rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart().ptr
						+ descriptor_handle_increment_size * back_buffer_index
				};

				command_list.OMSetRenderTargets(1, &render_target_descriptor, true, nullptr);

				std::array<FLOAT, 4> clear_color{ 0.0f, 0.0f, 1.0f, 1.0f };
				command_list.ClearRenderTargetView(render_target_descriptor, clear_color.data(), 0, nullptr);
			}

			command_list.SetGraphicsRootSignature(m_root_signature.get());

			{
				{
					D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;
					vertex_buffer_view.BufferLocation = m_triangle.vertex_buffer->GetGPUVirtualAddress();
					vertex_buffer_view.SizeInBytes = 8 * 4 * 3;
					vertex_buffer_view.StrideInBytes = 8 * 4;
					command_list.IASetVertexBuffers(0, 1, &vertex_buffer_view);
				}

				{
					D3D12_INDEX_BUFFER_VIEW index_buffer_view;
					index_buffer_view.BufferLocation = m_triangle.index_buffer->GetGPUVirtualAddress();
					index_buffer_view.SizeInBytes = 3 * 2;
					index_buffer_view.Format = DXGI_FORMAT_R16_UINT;
					command_list.IASetIndexBuffer(&index_buffer_view);
				}

				command_list.IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				command_list.DrawIndexedInstanced(3, 1, 0, 0, 0);
			}

			{
				D3D12_RESOURCE_BARRIER resource_barrier;
				resource_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				resource_barrier.Transition.pResource = back_buffer.get();
				resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
				resource_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
				resource_barrier.Transition.Subresource = 0;
				resource_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				command_list.ResourceBarrier(1, &resource_barrier);
			}

			winrt::check_hresult(
				command_list.Close());

			std::array<ID3D12CommandList*, 1> command_lists_to_execute
			{
				&command_list
			};

			m_direct_command_queue->ExecuteCommandLists(
				static_cast<UINT>(command_lists_to_execute.size()), command_lists_to_execute.data()
			);
		}

		{
			UINT64 const frame_finished_value = static_cast<UINT64>(m_submitted_frames);
			m_direct_command_queue->Signal(m_fence.get(), frame_finished_value);
			++m_submitted_frames;
		}
	}

	void Renderer::present()
	{
		winrt::check_hresult(
			m_swap_chain->Present(1, 0));
	}
}
