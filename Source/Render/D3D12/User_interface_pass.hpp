#ifndef MAIA_MYTHOLOGY_USERINTERFACEPASS_H_INCLUDED
#define MAIA_MYTHOLOGY_USERINTERFACEPASS_H_INCLUDED

#include <utility>

#include <d3d12.h>

#include <winrt/base.h>

#include <Maia/Renderer/D3D12/Heaps/Image_heap.hpp>
#include <Maia/Renderer/D3D12/Utilities/Sampled_image.hpp>
#include <Maia/Renderer/D3D12/Utilities/Upload_buffer.hpp>

namespace Maia::Mythology
{
	class User_interface_pass
	{
	public:

		User_interface_pass(
			ID3D12Device& device,
			ID3D12GraphicsCommandList& command_list,
			Maia::Renderer::D3D12::Upload_buffer_view upload_buffer_view,
			std::pair<float, float> display_size
		) noexcept;

		void upload_user_interface_data() noexcept;

		void execute_user_interface_pass() noexcept;


	private:

		std::pair<Maia::Renderer::D3D12::Image_heap, Maia::Renderer::D3D12::Sampled_image> m_font_texture;
	};
}

#endif
