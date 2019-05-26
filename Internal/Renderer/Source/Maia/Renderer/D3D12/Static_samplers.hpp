#ifndef MAIA_RENDERER_D3D12_STATICSAMPLERS_H_INCLUDED
#define MAIA_RENDERER_D3D12_STATICSAMPLERS_H_INCLUDED

#include <array>

#include <d3d12.h>

namespace Maia::Renderer::D3D12
{
	struct Static_sampler : public D3D12_STATIC_SAMPLER_DESC
	{
		constexpr Static_sampler(
			UINT shader_register,
			UINT shader_register_space,
			D3D12_FILTER filter = D3D12_FILTER_ANISOTROPIC,
			D3D12_TEXTURE_ADDRESS_MODE address_u = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE address_v = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE address_w = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			FLOAT mip_lod_bias = 0.0f,
			UINT max_anisotropy = 16,
			D3D12_COMPARISON_FUNC comparison_function = D3D12_COMPARISON_FUNC_LESS_EQUAL,
			D3D12_STATIC_BORDER_COLOR border_color = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
			FLOAT min_lod = 0.f,
			FLOAT max_lod = D3D12_FLOAT32_MAX,
			D3D12_SHADER_VISIBILITY shader_visibility = D3D12_SHADER_VISIBILITY_ALL
		) noexcept;
	};

	constexpr std::array<Static_sampler, 6> create_static_samplers() noexcept;
}

#endif
