#include <iostream>

#include <winerror.h>

#include <winrt/base.h>

#include "Check_hresult.hpp"

namespace Maia::Renderer::D3D12
{
	void check_hresult(HRESULT const result)
	{
		if (FAILED(result))
		{
			{
				winrt::hresult_error error{ result };
				std::cerr << winrt::to_string(error.message()) << '\n';
			}

			winrt::check_hresult(result);
		}
	}
}
