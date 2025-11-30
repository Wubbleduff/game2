
struct VS_INPUT
{
    float2 pos : POSITION0;
    float2 tex_coord : TEXCOORD0;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 tex_coord : TEXCOORD0;
};

Texture2D src_tex : register(t0);
SamplerState src_sampler : register(s0);

PS_INPUT vs(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = float4(input.pos, 0.0f, 1.0f);
    output.tex_coord = input.tex_coord;
    return output;
}

float4 ps(PS_INPUT input) : SV_TARGET
{
    // https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom
    
    uint tex_width;
    uint tex_height;
    uint tex_levels;
    src_tex.GetDimensions(0, tex_width, tex_height, tex_levels);

    float2 uv = input.tex_coord;

    // The filter kernel is applied with a radius, specified in texture
    // coordinates, so that the radius will vary across mip resolutions.
    const float filter_radius = 0.005;
    float x = filter_radius;
    float y = filter_radius;

    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    float3 a = src_tex.Sample(src_sampler, float2(uv.x - x, uv.y + y)).rgb;
    float3 b = src_tex.Sample(src_sampler, float2(uv.x,     uv.y + y)).rgb;
    float3 c = src_tex.Sample(src_sampler, float2(uv.x + x, uv.y + y)).rgb;

    float3 d = src_tex.Sample(src_sampler, float2(uv.x - x, uv.y)).rgb;
    float3 e = src_tex.Sample(src_sampler, float2(uv.x,     uv.y)).rgb;
    float3 f = src_tex.Sample(src_sampler, float2(uv.x + x, uv.y)).rgb;

    float3 g = src_tex.Sample(src_sampler, float2(uv.x - x, uv.y - y)).rgb;
    float3 h = src_tex.Sample(src_sampler, float2(uv.x,     uv.y - y)).rgb;
    float3 i = src_tex.Sample(src_sampler, float2(uv.x + x, uv.y - y)).rgb;

    // Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |
    float3 upsample = e*4.0;
    upsample += (b+d+f+h)*2.0;
    upsample += (a+c+g+i);
    upsample *= 1.0 / 16.0;

    return float4(upsample, 1.0);
}
