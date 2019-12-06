#ifndef MAIA_RENDERER_D3D12_CHECKHRESULT_H_INCLUDED
#define MAIA_RENDERER_D3D12_CHECKHRESULT_H_INCLUDED

typedef long HRESULT;

namespace Maia::Renderer::D3D12
{
	void check_hresult(HRESULT result);
}

#endif
