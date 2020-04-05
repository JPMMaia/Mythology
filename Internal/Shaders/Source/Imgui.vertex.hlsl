struct Vertex_shader_input
{
    float2 position : POSITION;
    float2 texture_coordinates : TEXCOORD;
    float4 color : COLOR;
};

struct Vertex_shader_output
{
    float4 clip_position : SV_POSITION;
    float4 color : COLOR;
    float2 texture_coordinates : TEXCOORD;
};

struct Transform
{
    float2 scale;
    float2 translation;
};

ConstantBuffer<Transform> g_transform : register(b0, space0);

Vertex_shader_output main(Vertex_shader_input const input)
{
    Vertex_shader_output output;

    output.clip_position = float4(g_transform.scale * input.position + g_transform.translation, 0.0, 1.0);
    output.color = input.color;
    output.texture_coordinates = input.texture_coordinates;

    return output;
}
