set (program_files_env "ProgramFiles(x86)")
find_program (dxc "dxc" PATHS "$ENV{${program_files_env}}/Windows Kits/10/bin/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/x64")
mark_as_advanced (dxc)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (DXC REQUIRED_VARS dxc)

if (DXC_FOUND AND NOT TARGET DirectXShaderCompiler::dxc)

    add_executable (DirectXShaderCompiler::dxc IMPORTED)

    set_target_properties (
        DirectXShaderCompiler::dxc
        PROPERTIES
            IMPORTED_LOCATION "${dxc}")

endif()
