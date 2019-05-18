#include <iostream>

#include <Maia/GameEngine/Component_group.hpp>
#include <Maia/GameEngine/Entity_manager.hpp>
#include <Maia/GameEngine/Entity_type.hpp>

#include <Maia/Renderer/D3D12/Utilities/Buffer_view.hpp>
#include <Maia/Renderer/D3D12/Utilities/Check_hresult.hpp>
#include <Maia/Renderer/D3D12/Utilities/D3D12_utilities.hpp>
#include <Maia/Renderer/D3D12/Utilities/Samplers.hpp>
#include <Maia/Renderer/Matrices.hpp>

#include <Components/Camera_component.hpp>
#include <Render/Pass_data.hpp>
#include <Render/D3D12/User_interface_pass.hpp>

#include "Render_system.hpp"
#include <Maia/GameEngine/Systems/Transform_system.hpp>

#include <d3dx12.h>


using namespace Maia::GameEngine;
using namespace Maia::GameEngine::Systems;
using namespace Maia::Renderer;
using namespace Maia::Renderer::D3D12;

namespace Maia::Mythology::D3D12
{
	namespace
	{
		std::size_t calculate_instance_buffer_size(
			Maia::GameEngine::Entity_manager const& entity_manager,
			gsl::span<Maia::GameEngine::Entity_type_id const> const entity_types_ids
		)
		{
			using namespace Maia::GameEngine;

			std::size_t count{ 0 };

			for (Maia::GameEngine::Entity_type_id const entity_type_id : entity_types_ids)
			{
				Component_group const& component_group = entity_manager.get_component_group(entity_type_id);

				count += component_group.size();
			}

			std::size_t const unaligned_size_in_bytes = count * sizeof(Instance_data);
			std::size_t constexpr alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

			std::size_t const num_blocks = unaligned_size_in_bytes / alignment
				+ (unaligned_size_in_bytes % alignment == 0 ? 0 : 1);

			return num_blocks * alignment;
		}

		std::vector<Instance_buffer> create_instance_buffers(
			ID3D12Device & device,
			ID3D12Heap & heap, UINT64 const heap_offset,
			UINT64 const size,
			std::size_t const count
		)
		{
			std::vector<Instance_buffer> instance_buffers;
			instance_buffers.reserve(count);

			for (std::size_t index = 0; index < count; ++index)
			{
				const UINT64 current_heap_offset = heap_offset +
					index * size;

				instance_buffers.push_back(
					{
						create_buffer(
							device,
							heap, current_heap_offset,
							size,
							D3D12_RESOURCE_STATE_COMMON
						)
					}
				);
			}

			return instance_buffers;
		}

		winrt::com_ptr<ID3D12Resource> create_depth_stencil_buffer(
			ID3D12Device & device,
			UINT64 const width, UINT const height,
			UINT16 const array_size, UINT16 const mip_levels
		)
		{
			CD3DX12_HEAP_PROPERTIES const heap_properties{ D3D12_HEAP_TYPE_DEFAULT };

			CD3DX12_RESOURCE_DESC const description = CD3DX12_RESOURCE_DESC::Tex2D(
				DXGI_FORMAT_D32_FLOAT,
				width, height,
				array_size, mip_levels,
				1, 0,
				D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
				D3D12_TEXTURE_LAYOUT_UNKNOWN,
				0
			);

			CD3DX12_CLEAR_VALUE const optimized_clear_value{
				DXGI_FORMAT_D32_FLOAT, 1.0f, 0
			};

			winrt::com_ptr<ID3D12Resource> depth_stencil_buffer;
			winrt::check_hresult(
				device.CreateCommittedResource(
					&heap_properties,
					D3D12_HEAP_FLAG_NONE,
					&description,
					D3D12_RESOURCE_STATE_DEPTH_WRITE,
					&optimized_clear_value,
					__uuidof(ID3D12Resource),
					depth_stencil_buffer.put_void()
				)
			);

			return depth_stencil_buffer;
		}

		struct Shader_index
		{
			enum Value : std::uint8_t
			{
				Position_vertex_shader = 0,
				Position_fragment_shader,
				Color_vertex_shader,
				Color_fragment_shader,
				User_interface_vertex_shader,
				User_interface_pixel_shader,
			};
		};

		std::vector<Maia::Renderer::D3D12::Shader> create_shaders()
		{
			std::vector<Maia::Renderer::D3D12::Shader> shaders;
			shaders.reserve(6);

			shaders.emplace_back(L"Shaders/Position_vertex_shader.cso");
			shaders.emplace_back(L"Shaders/Position_pixel_shader.cso");
			shaders.emplace_back(L"Shaders/Color_vertex_shader.cso");
			shaders.emplace_back(L"Shaders/Color_pixel_shader.cso");
			shaders.emplace_back(L"Shaders/User_interface_vertex_shader.cso");
			shaders.emplace_back(L"Shaders/User_interface_pixel_shader.cso");

			return shaders;
		}

		std::vector<winrt::com_ptr<ID3D12RootSignature>> create_root_signatures(ID3D12Device & device)
		{
			std::vector<winrt::com_ptr<ID3D12RootSignature>> root_signatures;
			root_signatures.reserve(3);

			std::array<CD3DX12_STATIC_SAMPLER_DESC, 6> const static_samplers = create_static_samplers();

			{
				std::array<CD3DX12_ROOT_PARAMETER1, 1> root_parameters;
				root_parameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);

				root_signatures.push_back(
					create_root_signature(device, root_parameters, {}, 0)
				);
			}

			{
				std::array<CD3DX12_ROOT_PARAMETER1, 1> root_parameters;
				root_parameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);

				root_signatures.push_back(
					create_root_signature(device, root_parameters, {}, 0)
				);
			}

			{
				std::array<CD3DX12_ROOT_PARAMETER1, 2> root_parameters;
				root_parameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);

				std::array<D3D12_DESCRIPTOR_RANGE1, 1> descriptor_ranges;
				descriptor_ranges[0] = CD3DX12_DESCRIPTOR_RANGE1{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC, 0 };

				root_parameters[1].InitAsDescriptorTable(
					static_cast<UINT>(descriptor_ranges.size()), descriptor_ranges.data(),
					D3D12_SHADER_VISIBILITY_PIXEL
				);

				root_signatures.push_back(
					create_root_signature(device, root_parameters, static_samplers, 0)
				);
			}

			return root_signatures;
		}

		CD3DX12_PIPELINE_STATE_STREAM1 create_default_pipeline_state_stream()
		{
			D3D12_GRAPHICS_PIPELINE_STATE_DESC description{};
			description.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			description.SampleMask = 0xFFFFFFFF;
			description.RasterizerState = CD3DX12_RASTERIZER_DESC{ D3D12_DEFAULT };
			description.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
			description.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC1{ D3D12_DEFAULT };
			description.DepthStencilState.StencilEnable = FALSE;
			description.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
			description.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			description.NumRenderTargets = 1;
			description.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			description.DSVFormat = DXGI_FORMAT_D32_FLOAT;
			description.SampleDesc.Count = 1;
			description.SampleDesc.Quality = 0;
			description.NodeMask;
			description.CachedPSO;
			description.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

			return { description };
		}

		std::vector<winrt::com_ptr<ID3D12PipelineState>> create_pipeline_states(
			ID3D12Device2 & device,
			gsl::span<Maia::Renderer::D3D12::Shader const> const shaders,
			gsl::span<winrt::com_ptr<ID3D12RootSignature> const> const root_signatures
		)
		{
			std::vector<winrt::com_ptr<ID3D12PipelineState>> pipeline_states;
			pipeline_states.reserve(3);

			CD3DX12_PIPELINE_STATE_STREAM1 const default_pipeline_state_stream =
				create_default_pipeline_state_stream();

			{
				D3D12_GRAPHICS_PIPELINE_STATE_DESC description =
					default_pipeline_state_stream.GraphicsDescV0();

				description.pRootSignature = root_signatures[0].get();
				description.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
				description.VS = shaders[Shader_index::Position_vertex_shader].bytecode();
				description.PS = shaders[Shader_index::Position_fragment_shader].bytecode();

				std::array<D3D12_INPUT_ELEMENT_DESC, 5> input_layout_elements
				{
					{
						{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

						{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
						{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
						{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
						{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
					}
				};
				description.InputLayout = { input_layout_elements.data(), static_cast<UINT>(input_layout_elements.size()) };

				winrt::com_ptr<ID3D12PipelineState> pipeline_state;
				winrt::check_hresult(
					device.CreateGraphicsPipelineState(&description, __uuidof(ID3D12PipelineState), pipeline_state.put_void()));

				pipeline_states.push_back(pipeline_state);
			}

			{
				D3D12_GRAPHICS_PIPELINE_STATE_DESC description =
					default_pipeline_state_stream.GraphicsDescV0();

				description.pRootSignature = root_signatures[1].get();
				description.VS = shaders[Shader_index::Color_vertex_shader].bytecode();
				description.PS = shaders[Shader_index::Color_fragment_shader].bytecode();

				std::array<D3D12_INPUT_ELEMENT_DESC, 6> input_layout_elements
				{
					{
						{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
						{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

						{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
						{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
						{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
						{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
					}
				};
				description.InputLayout = { input_layout_elements.data(), static_cast<UINT>(input_layout_elements.size()) };

				winrt::com_ptr<ID3D12PipelineState> pipeline_state;
				winrt::check_hresult(
					device.CreateGraphicsPipelineState(&description, __uuidof(ID3D12PipelineState), pipeline_state.put_void()));

				pipeline_states.push_back(pipeline_state);
			}

			{
				D3D12_GRAPHICS_PIPELINE_STATE_DESC description =
					default_pipeline_state_stream.GraphicsDescV0();

				description.pRootSignature = root_signatures[2].get();
				description.VS = shaders[Shader_index::User_interface_vertex_shader].bytecode();
				description.PS = shaders[Shader_index::User_interface_pixel_shader].bytecode();

				std::array<D3D12_INPUT_ELEMENT_DESC, 3> input_layout_elements
				{
					{
						{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
						{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
						{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					}
				};
				description.InputLayout = { input_layout_elements.data(), static_cast<UINT>(input_layout_elements.size()) };

				winrt::com_ptr<ID3D12PipelineState> pipeline_state;
				winrt::check_hresult(
					device.CreateGraphicsPipelineState(&description, __uuidof(ID3D12PipelineState), pipeline_state.put_void()));

				pipeline_states.push_back(pipeline_state);
			}

			return pipeline_states;
		}
	}

	Render_system::Render_system(
		ID3D12Device2& device,
		ID3D12CommandQueue& copy_command_queue,
		ID3D12CommandQueue& direct_command_queue,
		Swap_chain swap_chain,
		std::uint8_t const pipeline_length,
		bool const vertical_sync
	) :
		m_device{ device },
		m_copy_command_queue{ copy_command_queue },
		m_direct_command_queue{ direct_command_queue },
		m_swap_chain{ swap_chain.value },

		m_pipeline_length{ pipeline_length },
		m_vertical_sync{ vertical_sync },
		m_window_size{ swap_chain.bounds },
		m_copy_fence_value{ 0 },
		m_copy_fence{ create_fence(device, m_copy_fence_value, D3D12_FENCE_FLAG_NONE) },

		m_upload_frame_data_system{ device, m_pipeline_length },
		m_renderer{ device, swap_chain.bounds, m_pipeline_length },
		m_frames_resources{ device, m_pipeline_length },

		m_submitted_frames{ 0 },
		m_fence{ create_fence(device, m_submitted_frames, D3D12_FENCE_FLAG_NONE) },
		m_fence_event{ ::CreateEvent(nullptr, false, false, nullptr) },

		m_depth_stencil_buffer{ create_depth_stencil_buffer(device, static_cast<UINT64>(swap_chain.bounds(0)), static_cast<UINT>(swap_chain.bounds(1)), 1, 1) },

		m_global_upload_buffer{ device, 3 * D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT },

		m_pass_heap{ create_buffer_heap(device, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT) },
		m_pass_buffer{ create_buffer(device, *m_pass_heap, 0, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, D3D12_RESOURCE_STATE_COMMON) },
		m_pass_data_upload_buffer_view{ m_global_upload_buffer.view(0, m_pipeline_length * sizeof(Pass_data)) },

		m_instance_buffers_heap{ create_buffer_heap(device, m_pipeline_length * D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT) },
		m_instance_buffer_per_frame{ create_instance_buffers(device, *m_instance_buffers_heap, 0, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, m_pipeline_length) },
		m_instances_upload_buffer{ device, m_pipeline_length * D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT },

		m_shaders{ create_shaders() },
		m_root_signatures{ create_root_signatures(device) },
		m_pipeline_states{ create_pipeline_states(device, m_shaders, m_root_signatures) },
		m_viewport{ 0.0f, 0.0f, static_cast<FLOAT>(swap_chain.bounds(0)), static_cast<FLOAT>(swap_chain.bounds(1)), D3D12_MIN_DEPTH, D3D12_MAX_DEPTH },
		m_scissor_rect{ 0, 0, swap_chain.bounds(0), swap_chain.bounds(1) },

		m_copy_command_allocators{ create_command_allocators(device, D3D12_COMMAND_LIST_TYPE_COPY, m_pipeline_length) },
		m_copy_command_list{ create_closed_graphics_command_list(device, 0, D3D12_COMMAND_LIST_TYPE_COPY, *m_copy_command_allocators.front(), nullptr).as<ID3D12GraphicsCommandList4>() },
		m_direct_command_allocators{ create_command_allocators(device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pipeline_length) },
		m_direct_command_list{ create_opened_graphics_command_list(device, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, *m_direct_command_allocators.front(), nullptr).as<ID3D12GraphicsCommandList4>() },

		m_user_interface_pass{ device, *m_direct_command_list, m_global_upload_buffer.view(0, 3 * D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT), { static_cast<float>(swap_chain.bounds(0)), static_cast<float>(swap_chain.bounds(1)) } }
	{
		// TODO upload user interface

		create_swap_chain_rtvs(
			device,
			m_swap_chain,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			m_frames_resources.rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(),
			m_pipeline_length
		);

		create_depth_stencil_view(
			device, 
			*m_depth_stencil_buffer, 
			DXGI_FORMAT_D32_FLOAT, 
			m_frames_resources.dsv_descriptor_heap->GetCPUDescriptorHandleForHeapStart()
		);

		/*m_scene_resources = Maia::Mythology::load(m_entity_manager, *m_render_resources);
		m_scene_resources.camera.width_by_height_ratio = bounds.Width / bounds.Height;*/

		/*
		ID3D12GraphicsCommandList& command_list = *render_resources.command_list;
		check_hresult(
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

		*/
	}

	namespace
	{
		template <class T, class U>
		T align(T value, U alignment)
		{
			return ((value - 1) | (alignment - 1)) + 1;
		}

		Eigen::Matrix4f to_api_specific_perspective_matrix()
		{
			Eigen::Matrix4f value;
			value <<
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, -1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f;
			return value;
		}


		Pass_data create_pass_data(
			Entity_manager const& entity_manager,
			Entity const camera_entity,
			Eigen::Vector2i const window_size
		)
		{
			Pass_data pass_data;

			{
				// TODO problem with camera. Upside down.
				Transform_matrix const camera_transform =
					entity_manager.get_component_data<Transform_matrix>(camera_entity);

				pass_data.view_matrix = camera_transform.value.inverse();
			}

			{
				Maia::Utilities::glTF::Camera const camera =
					entity_manager.get_component_data<Camera_component>(camera_entity).value;

				if (camera.type == Maia::Utilities::glTF::Camera::Type::Orthographic)
				{
					const auto& orthographic = std::get<Maia::Utilities::glTF::Camera::Orthographic>(camera.projection);

					pass_data.projection_matrix =
						to_api_specific_perspective_matrix() *
						create_orthographic_projection_matrix(
							orthographic.horizontal_magnification,
							orthographic.vertical_magnification,
							orthographic.near_z,
							orthographic.far_z
						);
				}
				else
				{
					const auto& perspective = std::get<Maia::Utilities::glTF::Camera::Perspective>(camera.projection);

					float const aspect_ratio = [&]() -> float
					{
						if (perspective.aspect_ratio)
						{
							return *perspective.aspect_ratio;
						}
						else
						{
							return static_cast<float>(window_size(0)) / window_size(1);
						}
					}();

					if (perspective.far_z)
					{
						pass_data.projection_matrix =
							to_api_specific_perspective_matrix() *
							create_finite_perspective_projection_matrix(
								aspect_ratio,
								perspective.vertical_field_of_view,
								perspective.near_z,
								*perspective.far_z
							);
					}
					else
					{
						pass_data.projection_matrix =
							to_api_specific_perspective_matrix() *
							create_infinite_perspective_projection_matrix(
								aspect_ratio,
								perspective.vertical_field_of_view,
								perspective.near_z
							);
					}
				}
			}

			return pass_data;
		}

		void create_and_upload_pass_data(
			Entity_manager const& entity_manager,
			Entity const camera_entity,
			Eigen::Vector2i const window_size,
			ID3D12GraphicsCommandList & command_list,
			Upload_buffer_view const upload_buffer_view,
			Buffer_view const pass_data_buffer_view
		)
		{
			Pass_data pass_data = create_pass_data(
				entity_manager,
				camera_entity,
				window_size
			);

			upload_buffer_data<Pass_data>(
				command_list,
				pass_data_buffer_view,
				upload_buffer_view,
				{ &pass_data, 1 }
			);
		}

		void bind_pass_data(
			ID3D12GraphicsCommandList & command_list,
			ID3D12Resource & pass_buffer,
			std::uint8_t const current_frame_index
		)
		{
			D3D12_GPU_VIRTUAL_ADDRESS const pass_data_buffer_address =
				pass_buffer.GetGPUVirtualAddress() + current_frame_index * align(sizeof(Pass_data), 256);

			command_list.SetGraphicsRootConstantBufferView(0, pass_data_buffer_address);
		}


		std::vector<D3D12_VERTEX_BUFFER_VIEW> create_instance_buffers(
			Entity_manager const& entity_manager,
			gsl::span<Maia::GameEngine::Entity_type_id const> entity_types_ids,
			Buffer_view const instance_buffer_view
		)
		{
			using namespace Maia::GameEngine;
			using namespace Maia::Renderer::D3D12;

			std::vector<D3D12_VERTEX_BUFFER_VIEW> instance_buffer_views;
			instance_buffer_views.reserve(entity_types_ids.size());

			UINT64 current_size_in_bytes{ 0 };

			for (Entity_type_id const entity_type_id : entity_types_ids)
			{
				Component_group const& component_group = entity_manager.get_component_group(entity_type_id);

				UINT64 size_in_bytes{ 0 };

				for (std::size_t chunk_index = 0; chunk_index < component_group.num_chunks(); ++chunk_index)
				{
					using namespace Maia::GameEngine::Systems;

					gsl::span<Transform_matrix const> const transform_matrices =
						component_group.components<Transform_matrix>(chunk_index);

					size_in_bytes += transform_matrices.size_bytes();
				}

				D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;
				vertex_buffer_view.BufferLocation =
					instance_buffer_view.resource().GetGPUVirtualAddress() +
					instance_buffer_view.offset() + current_size_in_bytes;
				vertex_buffer_view.SizeInBytes = static_cast<UINT>(size_in_bytes);
				vertex_buffer_view.StrideInBytes = sizeof(Instance_data);
				instance_buffer_views.push_back(vertex_buffer_view);

				current_size_in_bytes += size_in_bytes;
			}

			return instance_buffer_views;
		}

		std::vector<D3D12_VERTEX_BUFFER_VIEW> create_and_upload_instance_buffers(
			ID3D12GraphicsCommandList & command_list,
			Entity_manager const& entity_manager,
			gsl::span<Maia::GameEngine::Entity_type_id const> const entity_types_ids,
			Buffer_view const instance_buffer_view,
			Upload_buffer_view const upload_buffer_view
		)
		{
			std::vector<D3D12_VERTEX_BUFFER_VIEW> vertex_buffer_views =
				create_instance_buffers(
					entity_manager,
					entity_types_ids,
					instance_buffer_view
				);

			{
				using namespace Maia::GameEngine;
				using namespace Maia::Renderer::D3D12;

				UINT64 current_size_in_bytes{ 0 };

				for (Entity_type_id const entity_type_id : entity_types_ids)
				{
					Component_group const& component_group = entity_manager.get_component_group(entity_type_id);

					for (std::size_t chunk_index = 0; chunk_index < component_group.num_chunks(); ++chunk_index)
					{
						using namespace Maia::GameEngine::Systems;

						gsl::span<Transform_matrix const> const transform_matrices =
							component_group.components<Transform_matrix>(chunk_index);

						upload_buffer_data(
							command_list,
							instance_buffer_view.resource(), instance_buffer_view.offset() + current_size_in_bytes,
							upload_buffer_view.resource(), upload_buffer_view.offset() + current_size_in_bytes,
							transform_matrices
						);

						current_size_in_bytes += transform_matrices.size_bytes();
					}
				}
			}

			return vertex_buffer_views;
		}

		void draw_color_instances(
			ID3D12GraphicsCommandList & command_list,
			gsl::span<D3D12_VERTEX_BUFFER_VIEW const> const instance_buffer_views,
			gsl::span<Mesh_ID const> const instance_buffer_mesh_indices,
			gsl::span<Mesh_view const> const mesh_views
		)
		{
			command_list.IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			for (std::ptrdiff_t instance_buffer_index = 0; instance_buffer_index < instance_buffer_views.size(); ++instance_buffer_index)
			{
				D3D12_VERTEX_BUFFER_VIEW const& instance_buffer_view = instance_buffer_views[instance_buffer_index];
				UINT const instance_count = instance_buffer_view.SizeInBytes / instance_buffer_view.StrideInBytes;

				if (instance_count > 0)
				{
					Mesh_ID const mesh_index = instance_buffer_mesh_indices[instance_buffer_index];
					Mesh_view const& mesh_view = mesh_views[mesh_index.value];

					for (Submesh_view const& submesh_view : mesh_view.submesh_views)
					{
						// TODO select vertex buffer views used by color shader...
						command_list.IASetVertexBuffers(
							0,
							static_cast<UINT>(submesh_view.vertex_buffer_views.size()),
							submesh_view.vertex_buffer_views.data()
						);

						command_list.IASetVertexBuffers(
							static_cast<UINT>(submesh_view.vertex_buffer_views.size()),
							1,
							&instance_buffer_view
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
		}

		void execute_color_pass(
			ID3D12GraphicsCommandList4& command_list,
			gsl::span<D3D12_RENDER_PASS_RENDER_TARGET_DESC const> const render_targets,
			std::optional<D3D12_RENDER_PASS_DEPTH_STENCIL_DESC const*> const depth,
			D3D12_RENDER_PASS_FLAGS const render_pass_flags,
			gsl::span<D3D12_VERTEX_BUFFER_VIEW const> const instance_buffer_views,
			gsl::span<Mesh_ID const> const instance_buffer_mesh_indices,
			gsl::span<Mesh_view const> const mesh_views
		)
		{
			/*command_list.BeginRenderPass(
				static_cast<UINT>(render_targets.size()), render_targets.data(),
				depth.value_or(nullptr),
				render_pass_flags
			);*/

			command_list.OMSetRenderTargets(1, &render_targets[0].cpuDescriptor, true, &(*depth)->cpuDescriptor);
			command_list.ClearRenderTargetView(
				render_targets[0].cpuDescriptor,
				render_targets[0].BeginningAccess.Clear.ClearValue.Color,
				0, nullptr
			);

			if (depth)
			{
				command_list.ClearDepthStencilView(
					(*depth)->cpuDescriptor,
					D3D12_CLEAR_FLAG_DEPTH,
					(*depth)->DepthBeginningAccess.Clear.ClearValue.DepthStencil.Depth,
					0,
					0, nullptr
				);
			}

			draw_color_instances(
				command_list,
				instance_buffer_views,
				instance_buffer_mesh_indices,
				mesh_views
			);

			//command_list.EndRenderPass();
		}


		void upload_user_interface_data()
		{
		}

		void draw_user_interface_data()
		{
		}

		void submit_to(
			ID3D12CommandQueue & command_queue,
			ID3D12CommandList & command_list
		)
		{
			std::array<ID3D12CommandList*, 1> command_lists_to_execute
			{
				&command_list
			};

			command_queue.ExecuteCommandLists(
				static_cast<UINT>(command_lists_to_execute.size()), command_lists_to_execute.data()
			);
		}

		void waits_for(
			ID3D12CommandQueue& command_queue_that_waits, 
			ID3D12CommandQueue& command_queue_to_wait_for,
			ID3D12Fence& fence,
			UINT64 const fence_value
		) noexcept
		{
			UINT64 const copy_fence_value_to_signal_and_wait = fence_value;
			command_queue_to_wait_for.Signal(&fence, copy_fence_value_to_signal_and_wait);
			command_queue_that_waits.Wait(&fence, copy_fence_value_to_signal_and_wait);
		}
	}

	void Render_system::render_frame(
		Maia::GameEngine::Entity_manager const& entity_manager,
		Maia::GameEngine::Entity const camera_entity,
		gsl::span<Maia::GameEngine::Entity_type_id const> const entity_types_with_mesh,
		gsl::span<Mesh_ID const> const entity_types_mesh_indices,
		gsl::span<Maia::Mythology::D3D12::Mesh_view const> const mesh_views
	)
	{
		assert(entity_types_with_mesh.size() == entity_types_mesh_indices.size());

		std::uint8_t const current_frame_index{ m_submitted_frames % m_pipeline_length };

		{
			// TODO check if it is needed to create new instance buffers
		}

		if (m_submitted_frames >= m_pipeline_length)
		{
			// TODO return to do other cpu work instead of waiting

			UINT64 const event_value_to_wait = m_submitted_frames - m_pipeline_length + 1;

			Maia::Renderer::D3D12::wait(
				m_direct_command_queue, *m_fence, m_fence_event.get(), event_value_to_wait, INFINITE);
		}

		{
			IDXGISwapChain3& swap_chain = m_swap_chain;

			UINT const back_buffer_index = swap_chain.GetCurrentBackBufferIndex();
			winrt::com_ptr<ID3D12Resource> back_buffer;
			check_hresult(
				swap_chain.GetBuffer(back_buffer_index, __uuidof(back_buffer), back_buffer.put_void()));

			std::array<D3D12_RENDER_PASS_RENDER_TARGET_DESC, 1> render_targets;
			render_targets[0] = [&]() -> D3D12_RENDER_PASS_RENDER_TARGET_DESC
			{
				UINT const descriptor_handle_increment_size =
					m_device.GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

				D3D12_CPU_DESCRIPTOR_HANDLE const render_target_descriptor_handle
				{
					m_frames_resources.rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart().ptr +
						descriptor_handle_increment_size * back_buffer_index
				};

				D3D12_RENDER_PASS_RENDER_TARGET_DESC description;
				description.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
				description.BeginningAccess.Clear.ClearValue.Color[0] = 0.0f;
				description.BeginningAccess.Clear.ClearValue.Color[1] = 0.0f;
				description.BeginningAccess.Clear.ClearValue.Color[2] = 0.0f;
				description.BeginningAccess.Clear.ClearValue.Color[3] = 0.0f;
				description.BeginningAccess.Clear.ClearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				description.cpuDescriptor = render_target_descriptor_handle;
				description.EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
				return description;
			}();

			D3D12_RENDER_PASS_DEPTH_STENCIL_DESC depth = [this]()-> D3D12_RENDER_PASS_DEPTH_STENCIL_DESC
			{
				D3D12_RENDER_PASS_DEPTH_STENCIL_DESC description;
				description.DepthBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
				description.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Depth = 1.0f;
				description.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Stencil = 0;
				description.DepthBeginningAccess.Clear.ClearValue.Format = DXGI_FORMAT_D32_FLOAT;
				description.cpuDescriptor = m_frames_resources.dsv_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
				description.DepthBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
				description.DepthEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
				description.StencilBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS;
				description.StencilEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS;
				return description;
			}();

			{
				ID3D12CommandAllocator& copy_command_allocator = *m_copy_command_allocators[current_frame_index];
				check_hresult(
					copy_command_allocator.Reset());

				ID3D12GraphicsCommandList4& copy_command_list = *m_copy_command_list;
				check_hresult(
					copy_command_list.Reset(&copy_command_allocator, nullptr));

				ID3D12CommandAllocator& direct_command_allocator = *m_direct_command_allocators[current_frame_index];
				check_hresult(
					direct_command_allocator.Reset());

				ID3D12GraphicsCommandList4& direct_command_list = *m_direct_command_list;
				check_hresult(
					direct_command_list.Reset(&direct_command_allocator, nullptr));

				create_and_upload_pass_data(
					entity_manager,
					camera_entity,
					m_window_size,
					copy_command_list,
					m_pass_data_upload_buffer_view.sub_view(current_frame_index * sizeof(Pass_data), sizeof(Pass_data)),
					{ *m_pass_buffer, current_frame_index * align(sizeof(Pass_data), 256), sizeof(Pass_data) }
				);

				const std::vector<D3D12_VERTEX_BUFFER_VIEW> instance_buffer_views = create_and_upload_instance_buffers(
					copy_command_list,
					entity_manager,
					entity_types_with_mesh,
					{ *m_instance_buffer_per_frame[current_frame_index].value, 0, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT },
					m_instances_upload_buffer.view(current_frame_index * D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT)
				);

				direct_command_list.RSSetViewports(1, &m_viewport);
				direct_command_list.RSSetScissorRects(1, &m_scissor_rect);

				direct_command_list.SetPipelineState(m_pipeline_states[0].get());
				direct_command_list.SetGraphicsRootSignature(m_root_signatures[0].get());

				bind_pass_data(
					direct_command_list,
					*m_pass_buffer,
					current_frame_index
				);

				{
					const CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
						back_buffer.get(),
						D3D12_RESOURCE_STATE_PRESENT,
						D3D12_RESOURCE_STATE_RENDER_TARGET
					);

					direct_command_list.ResourceBarrier(1, &barrier);
				}

				execute_color_pass(
					direct_command_list,
					render_targets, &depth,
					D3D12_RENDER_PASS_FLAG_NONE,
					instance_buffer_views,
					entity_types_mesh_indices,
					mesh_views
				);


				m_user_interface_pass.upload_user_interface_data();
				m_user_interface_pass.execute_user_interface_pass();

				{
					const CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
						back_buffer.get(),
						D3D12_RESOURCE_STATE_RENDER_TARGET,
						D3D12_RESOURCE_STATE_PRESENT
					);

					direct_command_list.ResourceBarrier(1, &barrier);
				}

				check_hresult(
					copy_command_list.Close());

				check_hresult(
					direct_command_list.Close());


				submit_to(m_copy_command_queue, copy_command_list);
				waits_for(m_direct_command_queue, m_copy_command_queue, *m_copy_fence, ++m_copy_fence_value);
				submit_to(m_direct_command_queue, direct_command_list);
			}
		}

		{
			UINT const sync_interval =
				m_vertical_sync ? 1 : 0;

			check_hresult(
				m_swap_chain.Present(sync_interval, 0));
		}

		{
			UINT64 const frame_finished_value = ++m_submitted_frames;

			check_hresult(
				m_direct_command_queue.Signal(m_fence.get(), frame_finished_value));
		}

		/*
		/*{
			// This should return a command list (use deferred future)
			{
				ID3D12GraphicsCommandList& copy_command_list;

				upload_pass_data(copy_command_list);

				// TODO solve this issue
				std::vector<D3D12_VERTEX_BUFFER_VIEW> const instance_buffer_vies =
					upload_instance_buffers(copy_command_list);
			}

			// This should return a command list (use deferred future)
			{
				ID3D12GraphicsCommandList& direct_command_list;

				bind_pass_data(direct_command_list);
				draw_color_instances(direct_command_list);
			}

			{
				{
					std::array<ID3D12CommandList*, 1> const command_lists_to_execute{ &copy_command_list };
					m_copy_command_queue.ExecuteCommandLists(
						static_cast<UINT>(command_lists_to_execute.size()), command_lists_to_execute.data());
				}

				{
					UINT64 const copy_fence_value_to_signal_and_wait = ++m_copy_fence_value;
					m_copy_command_queue.Signal(m_copy_fence.get(), copy_fence_value_to_signal_and_wait);
					m_direct_command_queue.Wait(m_copy_fence.get(), copy_fence_value_to_signal_and_wait);
				}

				{
					std::array<ID3D12CommandList*, 1> const command_lists_to_execute{ &direct_command_list };
					m_direct_command_queue.ExecuteCommandLists(
						static_cast<UINT>(command_lists_to_execute.size()), command_lists_to_execute.data());
				}
			}
		}

		// Between passes, we can decide to begin / resume render passes

		{
			// THis should return a command list
			{
				upload_user_interface_data();
			}

			// THis should return a command list
			{
				// Need to begin/end render pass

				draw_user_interface_data();
			}

			{
				{
					std::array<ID3D12CommandList*, 1> const command_lists_to_execute{ &copy_command_list };
					m_copy_command_queue.ExecuteCommandLists(
						static_cast<UINT>(command_lists_to_execute.size()), command_lists_to_execute.data());
				}

				{
					UINT64 const copy_fence_value_to_signal_and_wait = ++m_copy_fence_value;
					m_copy_command_queue.Signal(m_copy_fence.get(), copy_fence_value_to_signal_and_wait);
					m_direct_command_queue.Wait(m_copy_fence.get(), copy_fence_value_to_signal_and_wait);
				}

				{
					std::array<ID3D12CommandList*, 1> const command_lists_to_execute{ &direct_command_list };
					m_direct_command_queue.ExecuteCommandLists(
						static_cast<UINT>(command_lists_to_execute.size()), command_lists_to_execute.data());
				}
			}
		}*/
		/*
		std::vector<D3D12_VERTEX_BUFFER_VIEW> instance_buffer_views;
		{
			Upload_bundle bundle =
				m_upload_frame_data_system.reset(current_frame_index);


			{
				using namespace Maia::GameEngine::Systems;
				using namespace Maia::Renderer;
				using namespace Maia::Utilities::glTF;



				Pass_data pass_data;

				{
					// TODO problem with camera. Upside down.

					Transform_matrix const camera_transform =
						entity_manager.get_component_data<Transform_matrix>(camera_entity);

					pass_data.view_matrix = camera_transform.value.inverse();
				}

				{
					Maia::Utilities::glTF::Camera const camera =
						entity_manager.get_component_data<Camera_component>(camera_entity).value;

					if (camera.type == Maia::Utilities::glTF::Camera::Type::Orthographic)
					{
						const auto& orthographic = std::get<Maia::Utilities::glTF::Camera::Orthographic>(camera.projection);

						pass_data.projection_matrix =
							to_api_specific_perspective_matrix() *
							create_orthographic_projection_matrix(
								orthographic.horizontal_magnification,
								orthographic.vertical_magnification,
								orthographic.near_z,
								orthographic.far_z
							);
					}
					else
					{
						const auto& perspective = std::get<Maia::Utilities::glTF::Camera::Perspective>(camera.projection);

						float const aspect_ratio = [this, &perspective]() -> float
						{
							if (perspective.aspect_ratio)
							{
								return *perspective.aspect_ratio;
							}
							else
							{
								return static_cast<float>(m_window_size(0)) / m_window_size(1);
							}
						}();

						if (perspective.far_z)
						{
							pass_data.projection_matrix =
								to_api_specific_perspective_matrix() *
								create_finite_perspective_projection_matrix(
									aspect_ratio,
									perspective.vertical_field_of_view,
									perspective.near_z,
									*perspective.far_z
								);
						}
						else
						{
							pass_data.projection_matrix =
								to_api_specific_perspective_matrix() *
								create_infinite_perspective_projection_matrix(
									aspect_ratio,
									perspective.vertical_field_of_view,
									perspective.near_z
								);
						}
					}
				}

				ID3D12Resource& pass_buffer = *m_pass_buffer;
				m_upload_frame_data_system.upload_pass_data(
					bundle,
					pass_data,
					pass_buffer, current_frame_index * align(sizeof(Pass_data), 256)
				);
			}


			Instance_buffer const& instance_buffer =
				m_instance_buffer_per_frame[current_frame_index];

			instance_buffer_views =
				m_upload_frame_data_system.upload_instance_data(
					bundle,
					instance_buffer, 0,
					entity_manager,
					entity_types_with_mesh
				);

			{
				ID3D12CommandList& command_list =
					m_upload_frame_data_system.close(bundle);

				std::array<ID3D12CommandList*, 1> command_lists_to_execute
				{
					&command_list
				};

				m_copy_command_queue.ExecuteCommandLists(
					static_cast<UINT>(command_lists_to_execute.size()), command_lists_to_execute.data()
				);
			}
		}

		{
			UINT64 const copy_fence_value_to_signal_and_wait = ++m_copy_fence_value;
			m_copy_command_queue.Signal(m_copy_fence.get(), copy_fence_value_to_signal_and_wait);
			m_direct_command_queue.Wait(m_copy_fence.get(), copy_fence_value_to_signal_and_wait);
		}

		/*{
			IDXGISwapChain3& swap_chain = m_swap_chain;

			UINT const back_buffer_index = swap_chain.GetCurrentBackBufferIndex();
			winrt::com_ptr<ID3D12Resource> back_buffer;
			check_hresult(
				swap_chain.GetBuffer(back_buffer_index, __uuidof(back_buffer), back_buffer.put_void()));

			UINT const descriptor_handle_increment_size =
				m_device.GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

			D3D12_CPU_DESCRIPTOR_HANDLE const render_target_descriptor_handle
			{
				m_frames_resources.rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart().ptr +
					descriptor_handle_increment_size * back_buffer_index
			};

			{
				D3D12_GPU_VIRTUAL_ADDRESS const pass_data_buffer_address =
					m_pass_buffer->GetGPUVirtualAddress() + current_frame_index * align(sizeof(Pass_data), 256);

				ID3D12CommandList& command_list =
					m_renderer.render(
						current_frame_index,
						*back_buffer,
						render_target_descriptor_handle,
						depth_stencil_descriptor_handle,
						instance_buffer_views,
						entity_types_mesh_indices,
						mesh_views,
						pass_data_buffer_address
					);

				std::array<ID3D12CommandList*, 1> command_lists_to_execute
				{
					&command_list
				};

				m_direct_command_queue.ExecuteCommandLists(
					static_cast<UINT>(command_lists_to_execute.size()), command_lists_to_execute.data()
				);
			}

			{
				UINT const sync_interval =
					m_vertical_sync ? 1 : 0;

				check_hresult(
					m_swap_chain.Present(sync_interval, 0));
			}

			{
				UINT64 const frame_finished_value = ++m_submitted_frames;

				check_hresult(
					m_direct_command_queue.Signal(m_fence.get(), frame_finished_value));
			}
		}*/
	}

	void Render_system::wait()
	{
		UINT64 const event_value_to_signal_and_wait = m_submitted_frames + m_pipeline_length;
		signal_and_wait(m_direct_command_queue, *m_fence, m_fence_event.get(), event_value_to_signal_and_wait, INFINITE);
	}

	void Render_system::on_window_resized(Eigen::Vector2i new_size)
	{
		wait();

		{
			ID3D12CommandQueue& command_queue = m_direct_command_queue;

			std::vector<UINT> create_node_masks(m_pipeline_length, 1);
			std::vector<IUnknown*> command_queues(m_pipeline_length, &command_queue);

			resize_swap_chain_buffers_and_recreate_rtvs(
				m_swap_chain,
				create_node_masks,
				command_queues,
				new_size,
				m_device,
				m_frames_resources.rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart()
			);
		}

		m_window_size = new_size;

		m_renderer.resize_viewport_and_scissor_rects(new_size);

		//m_scene_resources.camera.width_by_height_ratio = size.Width / size.Height;
	}
}
