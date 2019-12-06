add_library (MaiaRendererD3D12)
add_library (Maia::Renderer::D3D12 ALIAS MaiaRendererD3D12)

target_link_libraries (MaiaRendererD3D12 PUBLIC D3D12 DXGI WindowsApp)

find_package (D3DX12 REQUIRED)
target_link_libraries (MaiaRendererD3D12 PUBLIC D3D12::D3DX12)

target_sources (MaiaRendererD3D12 
    PRIVATE
        "D3D12/Utilities/Check_hresult.hpp"
        "D3D12/Utilities/Check_hresult.cpp"
        "D3D12/Utilities/D3D12_utilities.hpp"
        "D3D12/Utilities/D3D12_utilities.cpp"
        "D3D12/Utilities/Mapped_memory.hpp"
        "D3D12/Utilities/Mapped_memory.cpp"
        "D3D12/Utilities/Shader.hpp"
        "D3D12/Utilities/Shader.cpp"
)