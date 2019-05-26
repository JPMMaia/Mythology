#include "Static_samplers.hpp"

#include <d3dx12.h>

namespace Maia::Renderer::D3D12
{
	constexpr Static_sampler::Static_sampler(
		UINT const shader_register,
		UINT const shader_register_space,
		D3D12_FILTER const filter,
		D3D12_TEXTURE_ADDRESS_MODE const address_u,
		D3D12_TEXTURE_ADDRESS_MODE const address_v,
		D3D12_TEXTURE_ADDRESS_MODE const address_w,
		FLOAT const mip_lod_bias,
		UINT const max_anisotropy,
		D3D12_COMPARISON_FUNC const comparison_function,
		D3D12_STATIC_BORDER_COLOR const border_color,
		FLOAT const min_lod,
		FLOAT const max_lod,
		D3D12_SHADER_VISIBILITY const shader_visibility
	) noexcept :
		D3D12_STATIC_SAMPLER_DESC{}
	{
		this->Filter = filter;
		this->AddressU = address_u;
		this->AddressV = address_v;
		this->AddressW = address_w;
		this->MipLODBias = mip_lod_bias;
		this->MaxAnisotropy = max_anisotropy;
		this->ComparisonFunc = comparison_function;
		this->BorderColor = border_color;
		this->MinLOD = min_lod;
		this->MaxLOD = max_lod;
		this->ShaderRegister = shader_register;
		this->RegisterSpace = shader_register_space;
		this->ShaderVisibility = shader_visibility;
	}

	constexpr std::array<Static_sampler, 6> create_static_samplers() noexcept
	{
		constexpr Static_sampler const point_wrap
		{
			0,
			0,
			D3D12_FILTER_MIN_MAG_MIP_POINT,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP
		};

		constexpr Static_sampler const point_clamp
		{
			1,
			0,
			D3D12_FILTER_MIN_MAG_MIP_POINT,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP
		};

		constexpr Static_sampler const linear_wrap
		{
			2,
			0,
			D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP
		};

		constexpr Static_sampler const linear_clamp
		{
			3,
			0,
			D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP
		};

		constexpr Static_sampler const anisotropic_wrap
		{
			4,
			0,
			D3D12_FILTER_ANISOTROPIC,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP
		};

		constexpr Static_sampler const anisotropic_clamp
		{
			5,
			0,
			D3D12_FILTER_ANISOTROPIC,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP
		};

		return
		{
			point_wrap, point_clamp,
			linear_wrap, linear_clamp,
			anisotropic_wrap, anisotropic_clamp
		};
	}
}
