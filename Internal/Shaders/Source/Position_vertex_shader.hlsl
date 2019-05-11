struct World_position
{
	float3 value : POSITION;
};

struct Instance_data
{
	float4x4 world_matrix : WORLD;
	uint instance_ID : SV_INSTANCEID;
};

struct Pass_data
{
	float4x4 view_matrix : VIEW;
	float4x4 projection_matrix : PROJECTION;
};

struct Vertex_shader_output
{
	float4 positionH : SV_POSITION;
};

ConstantBuffer<Pass_data> g_pass_data : register(b0, space0);

Vertex_shader_output main(World_position world_position, Color color, Instance_data instance_data)
{
	Vertex_shader_output output;

	const float4 positionW = mul(instance_data.world_matrix, float4(world_position.value, 1.0f));
	const float4 positionV = mul(g_pass_data.view_matrix, positionW);
	output.positionH = mul(g_pass_data.projection_matrix, positionV);

	return output;
}
