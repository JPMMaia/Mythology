struct Pixel_shader_output
{
	float4 color : SV_Target;
};

Pixel_shader_output main()
{
	Pixel_shader_output output;
	output.color = float4(0.8f, 0.8f, 0.8f, 1.0f);

	return output;
}
