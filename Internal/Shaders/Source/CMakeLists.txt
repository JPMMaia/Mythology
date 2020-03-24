project (MaiaShaders VERSION 0.0.1 LANGUAGES CXX)


add_library (MaiaShaders OBJECT)
add_library (Maia::Shaders ALIAS MaiaShaders)
set_target_properties (MaiaShaders PROPERTIES LINKER_LANGUAGE CXX)

find_package(DirectXShaderCompiler MODULE REQUIRED)


include (target_shader)

set (_shaders_output_directory "${CMAKE_CURRENT_BINARY_DIR}/shaders")

target_shader (MaiaShaders DirectXShaderCompiler::dxc
    INPUT "Triangle.vertex.hlsl"
    OUTPUT "Triangle.vertex.spv"
    OUTPUT_DIRECTORY "${_shaders_output_directory}"
    FLAGS "-spirv" "-T" "vs_6_0" "-E" "main"
)

target_shader (MaiaShaders DirectXShaderCompiler::dxc
    INPUT "White.fragment.hlsl"
    OUTPUT "White.fragment.spv"
    OUTPUT_DIRECTORY "${_shaders_output_directory}"
    FLAGS "-spirv" "-T" "ps_6_0" "-E" "main"
)

install (
    DIRECTORY "${_shaders_output_directory}/"
    DESTINATION "shaders"
    COMPONENT "shaders"
)
