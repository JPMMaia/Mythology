struct Vertex_shader_input
{
    float4 positionH : POSITION;
    float4 color : COLOR;
};

struct Vertex_shader_output
{
    float4 positionH : SV_POSITION;
    float4 color : COLOR;
};

Vertex_shader_output main(Vertex_shader_input input)
{
    Vertex_shader_output output;
    output.positionH = input.positionH;
    output.color = input.color;

    return output;
}
