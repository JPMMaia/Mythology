project (MaiaShaders VERSION 0.0.1 LANGUAGES CXX)


add_library (MaiaShaders OBJECT)
add_library (Maia::Shaders ALIAS MaiaShaders)
set_target_properties (MaiaShaders PROPERTIES LINKER_LANGUAGE CXX)

find_package(DirectXShaderCompiler MODULE REQUIRED)


include (target_shader)

target_shader (MaiaShaders DirectXShaderCompiler::dxc
    INPUT "Triangle.vertex.hlsl"
    OUTPUT "Triangle.vertex.spv"
    FLAGS "-spirv" "-T" "vs_6_0" "-E" "main"
)

target_shader (MaiaShaders DirectXShaderCompiler::dxc
    INPUT "White.fragment.hlsl"
    OUTPUT "White.fragment.spv"
    FLAGS "-spirv" "-T" "ps_6_0" "-E" "main"
)
