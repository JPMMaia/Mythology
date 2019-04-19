#ifndef MAIA_RENDERER_D3D12_SAMPLERS_H_INCLUDED
#define MAIA_RENDERER_D3D12_SAMPLERS_H_INCLUDED

#include <array>

struct CD3DX12_STATIC_SAMPLER_DESC;

namespace Maia::Renderer::D3D12
{
	std::array<CD3DX12_STATIC_SAMPLER_DESC, 6> create_static_samplers();
}

#endif
