struct Input_position_h
{
	float4 value : POSITION;
};

struct Input_color
{
	float4 value : COLOR;
};

struct Vertex_shader_output
{
    float4 positionH : SV_POSITION;
    float4 color : COLOR;
};

Vertex_shader_output main(Input_position_h positionH, Input_color color)
{
    Vertex_shader_output output;
    output.positionH = positionH.value;
    output.color = color.value;

    return output;
}
