struct Pixel_shader_input
{
    float4 positionH : SV_POSITION;
    float4 color : COLOR;
};

struct Pixel_shader_output
{
    float4 color : SV_Target;
};

Pixel_shader_output main(Pixel_shader_input input)
{
    Pixel_shader_output output;
    output.color = input.color;

    return output;
}
