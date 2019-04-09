include (FetchContent)

FetchContent_Declare (
    DirectXRaytracingBinaries
    URL "https://github.com/Microsoft/DirectX-Graphics-Samples/releases/download/v1.5-dxr/DirectXRaytracingBinaries1.5.zip"
)

FetchContent_GetProperties (DirectXRaytracingBinaries)
if (NOT directxraytracingbinaries_POPULATED)
	FetchContent_Populate (DirectXRaytracingBinaries)
endif ()


FetchContent_Declare (
    DirectXRaytracingSource
    URL "https://github.com/Microsoft/DirectX-Graphics-Samples/archive/v1.5-dxr.zip"
)

FetchContent_GetProperties (DirectXRaytracingSource)
if (NOT directxraytracingsource_POPULATED)
    FetchContent_Populate (DirectXRaytracingSource)
endif ()

find_path (D3D12RaytracingFallback_INCLUDE_DIR "D3D12RaytracingFallback.h" PATHS "${directxraytracingsource_SOURCE_DIR}/Libraries/D3D12RaytracingFallback/Include" NO_DEFAULT_PATH)
mark_as_advanced (D3D12RaytracingFallback_INCLUDE_DIR)

find_path (D3D12RaytracingFallback_LOCATION "dxrfallbackcompiler.dll" PATHS "${directxraytracingbinaries_SOURCE_DIR}" NO_DEFAULT_PATH)
mark_as_advanced (D3D12RaytracingFallback_LOCATION)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (D3D12RaytracingFallback
    REQUIRED_VARS 
        D3D12RaytracingFallback_INCLUDE_DIR
        D3D12RaytracingFallback_LOCATION 
)

if (D3D12RaytracingFallback_FOUND AND NOT TARGET D3D12::RaytracingFallback)
    
    add_library (D3D12::RaytracingFallback OBJECT IMPORTED)

    set_target_properties(D3D12::RaytracingFallback PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${D3D12RaytracingFallback_INCLUDE_DIR}"
        RUNTIME_DIRECTORY "${D3D12RaytracingFallback_LOCATION}"
    )
    
endif ()
