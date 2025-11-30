
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

    float x = 1.0 / float(tex_width);
    float y = 1.0 / float(tex_height);

    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    // === ('e' is the current texel) ===
    float3 a = src_tex.Sample(src_sampler, float2(uv.x - 2.0*x, uv.y + 2.0*y)).rgb;
    float3 b = src_tex.Sample(src_sampler, float2(uv.x,         uv.y + 2.0*y)).rgb;
    float3 c = src_tex.Sample(src_sampler, float2(uv.x + 2.0*x, uv.y + 2.0*y)).rgb;

    float3 d = src_tex.Sample(src_sampler, float2(uv.x - 2.0*x, uv.y)).rgb;
    float3 e = src_tex.Sample(src_sampler, float2(uv.x,         uv.y)).rgb;
    float3 f = src_tex.Sample(src_sampler, float2(uv.x + 2.0*x, uv.y)).rgb;

    float3 g = src_tex.Sample(src_sampler, float2(uv.x - 2.0*x, uv.y - 2.0*y)).rgb;
    float3 h = src_tex.Sample(src_sampler, float2(uv.x,         uv.y - 2.0*y)).rgb;
    float3 i = src_tex.Sample(src_sampler, float2(uv.x + 2.0*x, uv.y - 2.0*y)).rgb;

    float3 j = src_tex.Sample(src_sampler, float2(uv.x - x, uv.y + y)).rgb;
    float3 k = src_tex.Sample(src_sampler, float2(uv.x + x, uv.y + y)).rgb;
    float3 l = src_tex.Sample(src_sampler, float2(uv.x - x, uv.y - y)).rgb;
    float3 m = src_tex.Sample(src_sampler, float2(uv.x + x, uv.y - y)).rgb;

    // Apply weighted distribution:
    // 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
    // a,b,d,e * 0.125
    // b,c,e,f * 0.125
    // d,e,g,h * 0.125
    // e,f,h,i * 0.125
    // j,k,l,m * 0.5
    // This shows 5 square areas that are being sampled. But some of them overlap,
    // so to have an energy preserving downsample we need to make some adjustments.
    // The weights are the distributed, so that the sum of j,k,l,m (e.g.)
    // contribute 0.5 to the final color output. The code below is written
    // to effectively yield this sum. We get:
    // 0.125*5 + 0.03125*4 + 0.0625*4 = 1
    float3 downsample = e*0.125;
    downsample += (a+c+g+i)*0.03125;
    downsample += (b+d+f+h)*0.0625;
    downsample += (j+k+l+m)*0.125;

    return float4(downsample, 1.0);
}
