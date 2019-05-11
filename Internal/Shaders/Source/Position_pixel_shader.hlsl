struct Pixel_shader_input
{
	float4 positionH : SV_POSITION;
};

struct Pixel_shader_output
{
	float4 color : SV_Target;
};

Pixel_shader_output main(Pixel_shader_input input)
{
	Pixel_shader_output output;
	output.color = float4(0.8f, 0.8f, 0.8f, 1.0f);

	return output;
}
