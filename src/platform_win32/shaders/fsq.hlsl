
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

Texture2D shader_texture : register(t0);
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
    float4 sample_result = shader_texture.Sample(sample_type, input.tex_coord);
    float3 result_color = sample_result.xyz;

    float3 bloom_color = float3(0.0, 0.0, 0.0);
    float texture_width;
    float texture_height;
    shader_texture.GetDimensions(texture_width, texture_height);
    float weight[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };
    float2 tex_offset = float2(1.0 / texture_width, 1.0 / texture_height);
    for(int i = 1; i < 5; ++i)
    {
        bloom_color += shader_texture.Sample(sample_type, input.tex_coord + float2(tex_offset.x * i, 0.0)).rgb * weight[i];
        bloom_color += shader_texture.Sample(sample_type, input.tex_coord - float2(tex_offset.x * i, 0.0)).rgb * weight[i];
    }
    for(int j = 1; j < 5; ++j)
    {
        bloom_color += shader_texture.Sample(sample_type, input.tex_coord + float2(0.0f, tex_offset.y * j)).rgb * weight[j];
        bloom_color += shader_texture.Sample(sample_type, input.tex_coord - float2(0.0f, tex_offset.y * j)).rgb * weight[j];
    }

    if(length(result_color) > 0.2)
    {
        result_color += bloom_color;
    }

    float exposure = 3.0;
    result_color = float3(1.0, 1.0, 1.0) - exp(-result_color * (float3(1.0, 1.0, 1.0) * exposure));

    // float inv_gamma = 1.0 / 2.2;
    // result_color = pow(abs(result_color), float3(inv_gamma, inv_gamma, inv_gamma));

    return float4(result_color, sample_result.w);
}

