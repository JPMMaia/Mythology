#include "Root_signature.hpp"

#include <iostream>

#include <d3dx12.h>

#include <Maia/Renderer/D3D12/Utilities/Check_hresult.hpp>

namespace Maia::Renderer::D3D12
{
	namespace
	{
		[[nodiscard]] winrt::com_ptr<ID3D12RootSignature> create_root_signature(
			ID3D12Device& device,
			gsl::span<Root_signature_parameter const> const root_parameters,
			gsl::span<D3D12_STATIC_SAMPLER_DESC const> const static_samplers,
			UINT const node_mask,
			D3D12_ROOT_SIGNATURE_FLAGS const flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		) noexcept
		{
			D3D12_VERSIONED_ROOT_SIGNATURE_DESC versioned_description;
			versioned_description.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
			versioned_description.Desc_1_1 = [&]() -> D3D12_ROOT_SIGNATURE_DESC1
			{
				D3D12_ROOT_SIGNATURE_DESC1 description{};
				description.NumParameters = static_cast<UINT>(root_parameters.size());
				description.pParameters = root_parameters.data();
				description.NumStaticSamplers = static_cast<UINT>(static_samplers.size());
				description.pStaticSamplers = static_samplers.data();
				description.Flags = flags;
				return description;
			}();

			winrt::com_ptr<ID3DBlob> root_signature_blob;
			winrt::com_ptr<ID3DBlob> error_blob;
			{
				HRESULT const result = D3D12SerializeVersionedRootSignature(&versioned_description, root_signature_blob.put(), error_blob.put());

				if (FAILED(result))
				{
					if (error_blob)
					{
						std::wstring_view error_messages
						{
							reinterpret_cast<wchar_t*>(error_blob->GetBufferPointer()),
							static_cast<std::size_t>(error_blob->GetBufferSize())
						};

						std::cerr << error_messages.data();
					}

					check_hresult(result);
				}
			}

			winrt::com_ptr<ID3D12RootSignature> root_signature;
			check_hresult(
				device.CreateRootSignature(
					node_mask,
					root_signature_blob->GetBufferPointer(), root_signature_blob->GetBufferSize(),
					__uuidof(root_signature), root_signature.put_void()
				)
			);

			return root_signature;
		}
	}

	Root_signature::Root_signature(winrt::com_ptr<ID3D12RootSignature> root_signature) noexcept :
		value{ std::move(root_signature) }
	{
	}

	Root_signature::Root_signature(
		ID3D12Device& device,
		gsl::span<Root_signature_parameter const> const root_parameters,
		gsl::span<D3D12_STATIC_SAMPLER_DESC const> const static_samplers,
		D3D12_ROOT_SIGNATURE_FLAGS const flags
	) noexcept :
		value{ create_root_signature(device, root_parameters, static_samplers, 0, flags) }
	{
	}
}
