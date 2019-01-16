#include <array>
#include <iostream>

#include <Eigen/Eigen>
#include <gsl/gsl>

#include <d3dx12.h>

#include <Maia/Renderer/Matrices.hpp>
#include <Maia/Renderer/D3D12/Utilities/D3D12_utilities.hpp>
#include <Maia/Renderer/D3D12/Utilities/Mapped_memory.hpp>

#include <Renderer/Pass_data.hpp>

#include "Render_data.hpp"
#include "Renderer.hpp"

using namespace Maia::Renderer::D3D12;

namespace Maia::Mythology::D3D12
{
	namespace
	{
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

			std::array<D3D12_INPUT_ELEMENT_DESC, 6> input_layout_elements
			{
				D3D12_INPUT_ELEMENT_DESC
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

				{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
				{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
				{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 32, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
				{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 48, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
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

		winrt::com_ptr<ID3D12RootSignature> create_color_pass_root_signature(ID3D12Device5& device)
		{
			std::array<CD3DX12_ROOT_PARAMETER1, 1> root_parameters;
			root_parameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);

			return create_root_signature(device, root_parameters, {}, 0);
		}
	}

	Renderer::Renderer(IDXGIFactory6& factory, Render_resources& render_resources, Eigen::Vector2i viewport_and_scissor_dimensions, std::uint8_t pipeline_length) :
		m_pipeline_length{ pipeline_length },
		m_render_resources{ render_resources },
		m_fence_value{ 0 },
		m_fence{ create_fence(*render_resources.device, m_fence_value, D3D12_FENCE_FLAG_NONE) },
		m_fence_event{ ::CreateEvent(nullptr, false, false, nullptr) },
		m_viewport{ 0.0f, 0.0f, static_cast<FLOAT>(viewport_and_scissor_dimensions(0)), static_cast<FLOAT>(viewport_and_scissor_dimensions(1)), D3D12_MIN_DEPTH, D3D12_MAX_DEPTH },
		m_scissor_rect{ 0, 0, viewport_and_scissor_dimensions(0), viewport_and_scissor_dimensions(1) },
		m_submitted_frames{ m_pipeline_length },
		m_root_signature{ create_color_pass_root_signature(*m_render_resources.device) },
		m_color_vertex_shader{ "Resources/Shaders/Color_vertex_shader.csv" },
		m_color_pixel_shader{ "Resources/Shaders/Color_pixel_shader.csv" },
		m_color_pass_pipeline_state{ create_color_pass_pipeline_state(*m_render_resources.device, *m_root_signature, m_color_vertex_shader.bytecode(), m_color_pixel_shader.bytecode()) }
	{
		ID3D12GraphicsCommandList& command_list = *render_resources.command_list;
		winrt::check_hresult(
			command_list.Close());

		ID3D12CommandQueue& command_queue = *render_resources.direct_command_queue;
		{
			std::array<ID3D12CommandList*, 1> command_lists_to_execute
			{
				&command_list
			};

			command_queue.ExecuteCommandLists(
				static_cast<UINT>(command_lists_to_execute.size()), command_lists_to_execute.data()
			);
		}

		{
			UINT64 constexpr event_value_to_signal_and_wait = 1;
			signal_and_wait(command_queue, *m_fence, m_fence_event.get(), event_value_to_signal_and_wait, INFINITE);
		}
	}

	void Renderer::wait()
	{
		ID3D12CommandQueue& command_queue = *m_render_resources.direct_command_queue;

		UINT64 const event_value_to_signal_and_wait = m_submitted_frames + m_pipeline_length;
		signal_and_wait(command_queue, *m_fence, m_fence_event.get(), event_value_to_signal_and_wait, INFINITE);
	}

	void Renderer::resize_viewport_and_scissor_rects(Eigen::Vector2i window_dimensions)
	{
		m_viewport.Width = static_cast<FLOAT>(window_dimensions(0));
		m_viewport.Height = static_cast<FLOAT>(window_dimensions(1));
		m_scissor_rect.right = window_dimensions(0);
		m_scissor_rect.bottom = window_dimensions(1);
	}

	namespace
	{
		Eigen::Matrix4f create_api_specific_matrix()
		{
			Eigen::Matrix4f value;
			value <<
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, -1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f;
			return value;
		}

		void upload_data(
			ID3D12GraphicsCommandList& command_list, 
			ID3D12Resource& upload_buffer, 
			Scene_resources const& scene_resources,
			std::uint8_t const current_frame_index
		)
		{
			{
				Camera const& camera = scene_resources.camera;

				Pass_data pass_data;
				pass_data.view_matrix = Maia::Renderer::create_view_matrix(camera.position.value, camera.rotation.value);
				pass_data.projection_matrix =
					create_api_specific_matrix() *
					Maia::Renderer::create_perspective_projection_matrix(camera.vertical_half_angle_of_view, camera.width_by_height_ratio, camera.zRange);

				ID3D12Resource& constant_buffer = *scene_resources.constant_buffers[0];
				UINT64 const upload_buffer_offset = 1024 + 1024 * current_frame_index;;
				upload_buffer_data<Pass_data>(command_list, constant_buffer, 0, upload_buffer, upload_buffer_offset, { &pass_data, 1 });
			}
		}

		void draw(
			ID3D12GraphicsCommandList& command_list, 
			D3D12_VIEWPORT const& viewport, D3D12_RECT const& scissor_rect,
			ID3D12Resource& render_target, D3D12_CPU_DESCRIPTOR_HANDLE render_target_descriptor_handle,
			ID3D12RootSignature& root_signature,
			Scene_resources const& scene_resources
			)
		{
			command_list.RSSetViewports(1, &viewport);
			command_list.RSSetScissorRects(1, &scissor_rect);

			{
				D3D12_RESOURCE_BARRIER resource_barrier;
				resource_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				resource_barrier.Transition.pResource = &render_target;
				resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
				resource_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
				resource_barrier.Transition.Subresource = 0;
				resource_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				command_list.ResourceBarrier(1, &resource_barrier);
			}

			{
				command_list.OMSetRenderTargets(1, &render_target_descriptor_handle, true, nullptr);

				std::array<FLOAT, 4> clear_color{ 0.0f, 0.0f, 1.0f, 1.0f };
				command_list.ClearRenderTargetView(render_target_descriptor_handle, clear_color.data(), 0, nullptr);
			}

			command_list.SetGraphicsRootSignature(&root_signature);


			command_list.SetGraphicsRootConstantBufferView(0, scene_resources.constant_buffers[0]->GetGPUVirtualAddress());

			command_list.IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			for (std::size_t mesh_index = 0; mesh_index < scene_resources.mesh_views.size(); ++mesh_index)
			{
				Mesh_view const& mesh_view = scene_resources.mesh_views[mesh_index];
				UINT const instance_count = scene_resources.instances_count[mesh_index];

				if (instance_count > 0)
				{
					for (Submesh_view const& submesh_view : mesh_view.submesh_views)
					{
						command_list.IASetVertexBuffers(
							0,
							static_cast<UINT>(submesh_view.vertex_buffer_views.size()),
							submesh_view.vertex_buffer_views.data()
						);

						command_list.IASetIndexBuffer(
							&submesh_view.index_buffer_view
						);

						command_list.DrawIndexedInstanced(
							submesh_view.index_count,
							instance_count,
							0,
							0,
							0
						);
					}
				}
			}
			

			{
				D3D12_RESOURCE_BARRIER resource_barrier;
				resource_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				resource_barrier.Transition.pResource = &render_target;
				resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
				resource_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
				resource_barrier.Transition.Subresource = 0;
				resource_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				command_list.ResourceBarrier(1, &resource_barrier);
			}
		}
	}

	void Renderer::render(
		ID3D12Resource& render_target,
		D3D12_CPU_DESCRIPTOR_HANDLE render_target_descriptor_handle,
		Scene_resources const& scene_resources
	)
	{
		ID3D12Device& device = *m_render_resources.device;
		ID3D12CommandQueue& command_queue = *m_render_resources.direct_command_queue;

		{
			// TODO return to do other cpu work instead of waiting

			UINT64 const event_value_to_wait = m_submitted_frames - m_pipeline_length;
			Maia::Renderer::D3D12::wait(command_queue, *m_fence, m_fence_event.get(), event_value_to_wait, INFINITE);
		}

		{
			std::uint8_t current_frame_index{ m_submitted_frames % m_pipeline_length };
			ID3D12CommandAllocator& command_allocator = *m_render_resources.command_allocators[current_frame_index];

			winrt::check_hresult(
				command_allocator.Reset());

			{
				ID3D12GraphicsCommandList& command_list = *m_render_resources.command_list;
				winrt::check_hresult(
					command_list.Reset(&command_allocator, m_color_pass_pipeline_state.get()));

				upload_data(command_list, *m_render_resources.upload_buffer, scene_resources, current_frame_index);

				winrt::check_hresult(
					command_list.Close());

				std::array<ID3D12CommandList*, 1> command_lists_to_execute
				{
					&command_list
				};

				command_queue.ExecuteCommandLists(
					static_cast<UINT>(command_lists_to_execute.size()), command_lists_to_execute.data()
				);
			}

			{
				ID3D12GraphicsCommandList& command_list = *m_render_resources.command_list;
				winrt::check_hresult(
					command_list.Reset(&command_allocator, m_color_pass_pipeline_state.get()));

				draw(
					command_list,
					m_viewport, m_scissor_rect,
					render_target, render_target_descriptor_handle,
					*m_root_signature,
					scene_resources
				);

				winrt::check_hresult(
					command_list.Close());

				std::array<ID3D12CommandList*, 1> command_lists_to_execute
				{
					&command_list
				};

				command_queue.ExecuteCommandLists(
					static_cast<UINT>(command_lists_to_execute.size()), command_lists_to_execute.data()
				);
			}
		}

		{
			UINT64 const frame_finished_value = static_cast<UINT64>(m_submitted_frames);
			command_queue.Signal(m_fence.get(), frame_finished_value);
			++m_submitted_frames;
		}
	}
}
