struct Vertex_shader_output
{
	float4 clip_position : SV_POSITION;
};

static float2 const clip_positions[3] = {
    float2(0.0, -0.5),
    float2(0.5, 0.5),
    float2(-0.5, 0.5)
};

Vertex_shader_output main(uint const vertex_index : SV_VertexID)
{
	Vertex_shader_output output;

	output.clip_position = float4(clip_positions[vertex_index], 0.0, 1.0);

	return output;
}
