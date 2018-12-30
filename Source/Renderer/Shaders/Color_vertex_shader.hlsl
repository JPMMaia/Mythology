struct World_position
{
	float4 value : POSITION;
};

struct Color
{
	float4 value : COLOR;
};

struct Instance_data
{
	float4x4 world_matrix : WORLD;
	uint instance_ID : SV_INSTANCEID;
};

struct Vertex_shader_output
{
    float4 positionH : SV_POSITION;
    float4 color : COLOR;
};

Vertex_shader_output main(World_position world_position, Color color, Instance_data instance_data)
{
    Vertex_shader_output output;
	output.positionH = mul(instance_data.world_matrix, world_position.value);
    output.color = color.value;

    return output;
}
