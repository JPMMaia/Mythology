struct View_position
{
	float2 value : POSITION;
};

struct Color
{
	float4 value : COLOR0;
};

struct Texture_coordinate
{
	float2 value : TEXCOORD0;
};

struct Pass_data
{
	float4x4 projection_matrix : PROJECTION;
};

struct Vertex_shader_output
{
	float4 positionH : SV_POSITION;
	float4 color : COLOR0;
	float2 texture_coordinate : TEXCOORD0;
};

ConstantBuffer<Pass_data> g_pass_data : register(b0, space0);

Vertex_shader_output main(View_position view_position, Color color, Texture_coordinate texture_coordinate)
{
	Vertex_shader_output output;

	output.positionH = mul(g_pass_data.projection_matrix, float4(view_position.value, 0.0f, 1.0f));
	output.color = color.value;
	output.texture_coordinate = texture_coordinate.value;

	return output;
}
