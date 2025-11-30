
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

Texture2D hdr_tex : register(t0);
Texture2D bloom_tex : register(t1);
SamplerState sample_type : register(s0);

PS_INPUT vs(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = float4(input.pos, 0.0f, 1.0f);
    output.tex_coord = input.tex_coord;
    return output;
}

float4 ps(PS_INPUT input) : SV_TARGET
{
    float3 hdr_color = hdr_tex.Sample(sample_type, input.tex_coord).xyz;
    float3 bloom_color = bloom_tex.Sample(sample_type, input.tex_coord).xyz;

    float3 result_color = lerp(hdr_color, bloom_color, 0.06);

    float exposure = 3.0;
    hdr_color = float3(1.0, 1.0, 1.0) - exp(-result_color * (float3(1.0, 1.0, 1.0) * exposure));

    // float inv_gamma = 1.0 / 2.2;
    // result_color = pow(abs(result_color), float3(inv_gamma, inv_gamma, inv_gamma));

    return float4(result_color, 1.0);
}

