set (program_files_env "ProgramFiles(x86)")
find_program (Dxc_exe "dxc" PATHS "$ENV{${program_files_env}}/Windows Kits/10/bin/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/x64")
mark_as_advanced (Dxc_exe)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (DXC REQUIRED_VARS Dxc_exe)

if (DXC_FOUND AND NOT TARGET DirectXShaderCompiler::Dxc)

    add_library (DirectXShaderCompiler::Dxc INTERFACE IMPORTED)

endif()
