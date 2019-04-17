#include "Common/Static_samplers.hlsli"

struct Pixel_shader_input
{
	float4 positionH : SV_POSITION;
	float4 color : COLOR0;
	float2 texture_coordinate : TEXCOORD0;
};

struct Pixel_shader_output
{
	float4 color : SV_Target;
};

Texture2D g_texture : register(t0);

Pixel_shader_output main(Pixel_shader_input input)
{
	Pixel_shader_output output;
	output.color = input.color * g_texture.Sample(g_sampler_linear_wrap, input.texture_coordinate);

	return output;
}
