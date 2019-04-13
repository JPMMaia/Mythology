project (MaiaShaders VERSION 0.0.1 LANGUAGES CXX)


add_library (CopyShaders OBJECT)
add_library (Maia::CopyShaders ALIAS CopyShaders)
set_target_properties (CopyShaders PROPERTIES LINKER_LANGUAGE CXX)


add_library (CompileShaders OBJECT)
add_library (Maia::CompileShaders ALIAS CompileShaders)
set_target_properties (CompileShaders PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (CompileShaders PRIVATE CopyShaders)

find_package (DirectXShaderCompiler REQUIRED)
target_link_libraries (CompileShaders PRIVATE DirectXShaderCompiler::Dxc)



include (target_shader)


target_shader (CopyShaders CompileShaders
    INPUT "Color_vertex_shader.hlsl"
    TYPE "Vertex"
    INTERMEDIATE "Color_vertex_shader.hlsl"
)

target_shader (CopyShaders CompileShaders
    INPUT "Color_pixel_shader.hlsl"
    TYPE "Pixel"
    INTERMEDIATE "Color_pixel_shader.hlsl"
)
