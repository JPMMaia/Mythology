#ifndef MAIA_RENDERER_DESCRIPTOR_H_INCLUDED
#define MAIA_RENDERER_DESCRIPTOR_H_INCLUDED

#include <tuple>

#include <d3d12.h>

#include <Maia/Renderer/D3D12/Resource_view.hpp>

namespace Maia::Renderer::D3D12
{
	struct Descriptor
	{
	};

	
	struct Root_descriptor_slot : public Descriptor
	{
		D3D12_GPU_VIRTUAL_ADDRESS buffer_location;
	};

	struct Constant_buffer_root_descriptor_slot : public Root_descriptor_slot {};
	struct Shader_resource_root_descriptor_slot : public Root_descriptor_slot {};
	struct Unordered_access_root_descriptor_slot : public Root_descriptor_slot {};

	struct Descriptor_table_base_cpu_descriptor
	{
		D3D12_CPU_DESCRIPTOR_HANDLE value;
	};

	struct Descriptor_table_base_gpu_descriptor
	{
		D3D12_GPU_DESCRIPTOR_HANDLE value;
	};

	struct Cbv_srv_uav_descriptor_handle_increment_size
	{
		UINT64 value;

		Cbv_srv_uav_descriptor_handle_increment_size(
			ID3D12Device& device
		) noexcept :
			value{ device.GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) }
		{
		}
	};


	struct Descriptor_table : Descriptor
	{
		Descriptor_table_base_gpu_descriptor base_gpu_descriptor;
	};

	struct Shader_resource_2d_descriptor
	{
		Shader_resource_image_2d_view image_view;
		DXGI_FORMAT view_format;
	};

	void create_descriptor(
		ID3D12Device& device,
		D3D12_CPU_DESCRIPTOR_HANDLE const destinaton_desscriptor,
		Shader_resource_2d_descriptor const descriptor
	) noexcept
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC description;
		description.Format = descriptor.view_format;
		description.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		description.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		description.Texture2D = [&]() -> D3D12_TEX2D_SRV
		{
			D3D12_TEX2D_SRV texture2D;
			texture2D.MostDetailedMip = descriptor.image_view.first_mip_level.value;
			texture2D.MipLevels = descriptor.image_view.mip_levels.value;
			texture2D.PlaneSlice = 0;
			texture2D.ResourceMinLODClamp = 0.0f;
			return texture2D;
		}();

		device.CreateShaderResourceView(descriptor.image_view.image.value.get(), &description, destinaton_desscriptor);
	}


	template <class Last_descriptor>
	void create_descriptors(
		ID3D12Device& device,
		D3D12_CPU_DESCRIPTOR_HANDLE const destination_descriptor,
		Cbv_srv_uav_descriptor_handle_increment_size const descriptor_handle_increment_size,
		Last_descriptor const last_descriptor
	) noexcept
	{
		create_descriptor(device, destination_descriptor, last_descriptor);
	}

	template <class First_descriptor, class... Rest_descriptors>
	void create_descriptors(
		ID3D12Device& device,
		D3D12_CPU_DESCRIPTOR_HANDLE const base_descriptor,
		Cbv_srv_uav_descriptor_handle_increment_size const descriptor_handle_increment_size,
		First_descriptor const first_descriptor, Rest_descriptors const... rest_descriptors
		) noexcept
	{
		create_descriptor(device, base_descriptor, first_descriptor);

		create_descriptors(device, { base_descriptor.ptr + descriptor_handle_increment_size.value }, descriptor_handle_increment_size, rest_descriptors...);
	}

	

	struct Cbv_srv_uav_descriptor_table : public Descriptor_table 
	{
		template <class... Descriptors>
		Cbv_srv_uav_descriptor_table(
			ID3D12Device& device,
			Descriptor_table_base_cpu_descriptor const base_cpu_descriptor,
			Descriptor_table_base_gpu_descriptor const base_gpu_descriptor,
			Cbv_srv_uav_descriptor_handle_increment_size const descriptor_handle_increment_size,
			std::tuple<Descriptors...> const descriptors
		) noexcept : 
			Descriptor_table{}
		{
			this->base_gpu_descriptor = base_gpu_descriptor;

			auto const create_descriptors_with_args = [&](auto... descriptors) noexcept -> void
			{
				create_descriptors(device, base_cpu_descriptor.value, descriptor_handle_increment_size, descriptors...);
			};

			std::apply(create_descriptors_with_args, descriptors);
		}
	};

	struct Sampler_descriptor_table : public Descriptor_table {};
}

#endif