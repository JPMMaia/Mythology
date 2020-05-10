struct Pixel_shader_input
{
    float4 color : COLOR;
    float2 texture_coordinates : TEXCOORD;
};

struct Pixel_shader_output
{
	float4 color : SV_Target;
};

SamplerState const s_linear_wrap_sampler : register(s0, space0);

Texture2D g_texture : register(t1, space0);

Pixel_shader_output main(Pixel_shader_input const input)
{
    float const texture_color = g_texture.Sample(s_linear_wrap_sampler, input.texture_coordinates).r;

    Pixel_shader_output output;
	output.color = texture_color * input.color;

	return output;
}
