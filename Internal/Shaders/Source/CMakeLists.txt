project (MaiaShaders VERSION 0.0.1 LANGUAGES CXX)


add_library (CopyShaders OBJECT)
add_library (Maia::CopyShaders ALIAS CopyShaders)
set_target_properties (CopyShaders PROPERTIES LINKER_LANGUAGE CXX)


add_library (CompileShaders OBJECT)
add_library (Maia::CompileShaders ALIAS CompileShaders)
set_target_properties (CompileShaders PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (CompileShaders PRIVATE CopyShaders)

find_package(directx-shader-compiler CONFIG REQUIRED)

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/spirv_bin_file.spirv"
                   COMMAND directx-shader-compiler::dxc -spirv -T vs_6_0 -E main "${CMAKE_CURRENT_LIST_DIR}/Color_vertex_shader.hlsl" -Fo "${CMAKE_CURRENT_BINARY_DIR}/spirv_bin_file"
                   MAIN_DEPENDENCY "${CMAKE_CURRENT_LIST_DIR}/Color_vertex_shader.hlsl"
)
add_custom_target(hlsl-to-spirv
                  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/spirv_bin_file.spirv"
                  VERBATIM
)



add_library (Shaders INTERFACE)
add_library (Maia::Shaders ALIAS Shaders)
target_link_libraries (Shaders INTERFACE CompileShaders)



include (target_shader)


target_shader (CopyShaders CompileShaders
    INPUT "Color_vertex_shader.hlsl"
    TYPE "Vertex"
    INTERMEDIATE "Color_vertex_shader.hlsl" MODEL 5.1
)

target_shader (CopyShaders CompileShaders
    INPUT "Color_pixel_shader.hlsl"
    TYPE "Pixel"
    INTERMEDIATE "Color_pixel_shader.hlsl" MODEL 5.1
)