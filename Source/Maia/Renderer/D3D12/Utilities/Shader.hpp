#ifndef MAIA_RENDERER_SHADER_H_INCLUDED
#define MAIA_RENDERER_SHADER_H_INCLUDED

#include <filesystem>
#include <string_view>

#include <d3d12.h>

namespace Maia::Renderer::D3D12
{
	struct Shader
	{
		winrt::com_ptr<ID3DBlob> shader_blob;

		explicit Shader(std::filesystem::path const& compiled_shader_path);
		Shader(std::filesystem::path const& shader_path, std::string_view entry_point, std::string_view target);

		D3D12_SHADER_BYTECODE bytecode() const;
	};
}

#endif