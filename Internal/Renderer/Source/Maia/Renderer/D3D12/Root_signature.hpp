#ifndef MAIA_RENDERER_ROOTSIGNATURE_H_INCLUDED
#define MAIA_RENDERER_ROOTSIGNATURE_H_INCLUDED

#include <d3d12.h>

#include <winrt/base.h>

#include <gsl/span>

namespace Maia::Renderer::D3D12
{
	struct Shader_register
	{
		UINT value;
	};

	struct Shader_register_space
	{
		UINT value;
	};

	struct Num_32_bits_values
	{
		UINT value;
	};


	struct Root_signature_parameter : public D3D12_ROOT_PARAMETER1
	{
	};

	struct Root_32_bits_constant_root_signature_parameter : public Root_signature_parameter
	{
		constexpr Root_32_bits_constant_root_signature_parameter(
			Shader_register const shader_register,
			Shader_register_space const shader_register_space,
			Num_32_bits_values const num_32_bits_values,
			D3D12_SHADER_VISIBILITY const shader_visibility
		) noexcept :
			Root_signature_parameter{}
		{
			this->ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			this->Constants = { shader_register.value, shader_register_space.value, num_32_bits_values.value };
			this->ShaderVisibility = shader_visibility;
		}
	};

	struct Root_descriptor_root_signature_parameter : public Root_signature_parameter
	{
		constexpr Root_descriptor_root_signature_parameter(
			D3D12_ROOT_PARAMETER_TYPE const type,
			Shader_register const shader_register,
			Shader_register_space const register_space,
			D3D12_ROOT_DESCRIPTOR_FLAGS const flags,
			D3D12_SHADER_VISIBILITY const shader_visibility
		) noexcept :
			Root_signature_parameter{}
		{
			this->ParameterType = type;
			this->Descriptor = { shader_register.value, register_space.value, flags };
			this->ShaderVisibility = shader_visibility;
		}
	};

	struct Constant_buffer_view_root_descriptor_root_signature_parameter final : public Root_descriptor_root_signature_parameter
	{
		constexpr Constant_buffer_view_root_descriptor_root_signature_parameter(
			Shader_register const shader_register,
			Shader_register_space const register_space,
			D3D12_ROOT_DESCRIPTOR_FLAGS const flags,
			D3D12_SHADER_VISIBILITY const shader_visibility
		) noexcept :
			Root_descriptor_root_signature_parameter{ D3D12_ROOT_PARAMETER_TYPE_CBV, shader_register, register_space, flags, shader_visibility }
		{
		}
	};

	struct Shader_resource_view_root_descriptor_root_signature_parameter final : public Root_descriptor_root_signature_parameter
	{
		constexpr Shader_resource_view_root_descriptor_root_signature_parameter(
			Shader_register const shader_register,
			Shader_register_space const register_space,
			D3D12_ROOT_DESCRIPTOR_FLAGS const flags,
			D3D12_SHADER_VISIBILITY const shader_visibility
		) noexcept :
			Root_descriptor_root_signature_parameter{ D3D12_ROOT_PARAMETER_TYPE_SRV, shader_register, register_space, flags, shader_visibility }
		{
		}
	};

	struct Unordered_access_view_root_descriptor_root_signature_parameter final : public Root_descriptor_root_signature_parameter
	{
		constexpr Unordered_access_view_root_descriptor_root_signature_parameter(
			Shader_register const shader_register,
			Shader_register_space const register_space,
			D3D12_ROOT_DESCRIPTOR_FLAGS const flags,
			D3D12_SHADER_VISIBILITY const shader_visibility
		) noexcept :
			Root_descriptor_root_signature_parameter{ D3D12_ROOT_PARAMETER_TYPE_UAV, shader_register, register_space, flags, shader_visibility }
		{
		}
	};


	struct Descriptor_range_size
	{
		UINT value;
	};

	struct Descriptor_table_offset_in_descriptors
	{
		UINT value;
	};

	struct Descriptor_range : public D3D12_DESCRIPTOR_RANGE1
	{
		constexpr Descriptor_range(
			D3D12_DESCRIPTOR_RANGE_TYPE const type,
			Descriptor_range_size const num_descriptors,
			Shader_register const base_shader_register,
			Shader_register_space const register_space,
			D3D12_DESCRIPTOR_RANGE_FLAGS const flags,
			Descriptor_table_offset_in_descriptors const offset_in_descriptors_from_table_start
		) noexcept :
			D3D12_DESCRIPTOR_RANGE1{ type, num_descriptors.value, base_shader_register.value, register_space.value, flags, offset_in_descriptors_from_table_start.value }
		{
		}
	};

	struct Constant_buffer_view_descriptor_range : public Descriptor_range
	{
		constexpr Constant_buffer_view_descriptor_range(
			Descriptor_range_size const num_descriptors,
			Shader_register const base_shader_register,
			Shader_register_space const register_space,
			D3D12_DESCRIPTOR_RANGE_FLAGS const flags,
			Descriptor_table_offset_in_descriptors const offset_in_descriptors_from_table_start
		) noexcept :
			Descriptor_range{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, num_descriptors, base_shader_register, register_space, flags, offset_in_descriptors_from_table_start }
		{
		}
	};

	struct Shader_resource_view_descriptor_range : public Descriptor_range
	{
		constexpr Shader_resource_view_descriptor_range(
			Descriptor_range_size const num_descriptors,
			Shader_register const base_shader_register,
			Shader_register_space const register_space,
			D3D12_DESCRIPTOR_RANGE_FLAGS const flags,
			Descriptor_table_offset_in_descriptors const offset_in_descriptors_from_table_start
		) noexcept :
			Descriptor_range{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, num_descriptors, base_shader_register, register_space, flags, offset_in_descriptors_from_table_start }
		{
		}
	};

	struct Unordered_access_view_descriptor_range : public Descriptor_range
	{
		constexpr Unordered_access_view_descriptor_range(
			Descriptor_range_size const num_descriptors,
			Shader_register const base_shader_register,
			Shader_register_space const register_space,
			D3D12_DESCRIPTOR_RANGE_FLAGS const flags,
			Descriptor_table_offset_in_descriptors const offset_in_descriptors_from_table_start
		) noexcept :
			Descriptor_range{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, num_descriptors, base_shader_register, register_space, flags, offset_in_descriptors_from_table_start }
		{
		}
	};

	
	struct Descriptor_table_root_signature_parameter : public Root_signature_parameter
	{
		constexpr Descriptor_table_root_signature_parameter(
			gsl::span<Descriptor_range const> const descriptor_ranges,
			D3D12_SHADER_VISIBILITY const shader_visibility
		) noexcept :
			Root_signature_parameter{}
		{
			this->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			this->DescriptorTable = { static_cast<UINT>(descriptor_ranges.size()), descriptor_ranges.data() };
			this->ShaderVisibility = shader_visibility;
		}
	};

	struct Cbv_srv_uav_descriptor_table_root_signature_parameter final : public Descriptor_table_root_signature_parameter
	{
		constexpr Cbv_srv_uav_descriptor_table_root_signature_parameter(
			gsl::span<Descriptor_range const> const descriptor_ranges,
			D3D12_SHADER_VISIBILITY const shader_visibility
		) noexcept :
			Descriptor_table_root_signature_parameter{ descriptor_ranges, shader_visibility }
		{
		}
	};

	struct Sampler_descriptor_table_root_signature_parameter final : public Descriptor_table_root_signature_parameter
	{
		constexpr Sampler_descriptor_table_root_signature_parameter(
			gsl::span<Descriptor_range const> const descriptor_ranges,
			D3D12_SHADER_VISIBILITY const shader_visibility
		) noexcept :
			Descriptor_table_root_signature_parameter{ descriptor_ranges, shader_visibility }
		{
		}
	};


	struct Root_signature
	{
		winrt::com_ptr<ID3D12RootSignature> value;

		Root_signature(winrt::com_ptr<ID3D12RootSignature> root_signature) noexcept;

		Root_signature(
			ID3D12Device& device,
			gsl::span<Root_signature_parameter const> const root_parameters,
			gsl::span<D3D12_STATIC_SAMPLER_DESC const> const static_samplers,
			D3D12_ROOT_SIGNATURE_FLAGS const flags
		) noexcept;
	};
}

#endif
