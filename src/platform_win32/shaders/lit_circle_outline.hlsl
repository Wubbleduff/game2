
struct VS_INPUT
{
    float2 pos : POSITION0;
    float2 tex_coord : TEXCOORD0;

    uint instance_id : SV_InstanceID;
};

struct PS_INPUT
{
    float4 pos   : SV_POSITION;
    float4 color : COLOR0;
    float2 model_pos : POSITION4;
};

cbuffer cbuffer0 : register(b0)
{
    float4x4 utransform;
}

struct InstanceData
{
    float4 xform[3];
    float4 color;
};

StructuredBuffer<InstanceData> instance_data : register(t0);

PS_INPUT vs(VS_INPUT input)
{
    PS_INPUT output;
    InstanceData inst = instance_data[input.instance_id];
    float4 res = float4(
        dot(float4(input.pos, 0.0f, 1.0f), inst.xform[0]),
        dot(float4(input.pos, 0.0f, 1.0f), inst.xform[1]),
        dot(float4(input.pos, 0.0f, 1.0f), inst.xform[2]),
        1.0f);
    output.pos = mul(utransform, res);
    output.color = inst.color;
    output.model_pos = input.pos;
    return output;
}

float4 ps(PS_INPUT input) : SV_TARGET
{
    const float3 high_color = input.color.xyz;
    const float3 low_color = input.color.xyz * 0.02;
    float l = length(input.model_pos);
    float e_min = 0.4;
    float inner = 0.425;
    float outer = 0.475;
    float e_max = 0.5;
    float3 result_color = float3(0.0, 0.0, 0.0);
    if(l < e_min)
    {
        result_color = low_color;
    }
    else if(l >= e_min && l < inner)
    {
        float t = (l - e_min) / (inner - e_min);
        t = 3.0 * t*t - 2.0*t * t*t;
        result_color = lerp(low_color, high_color, float3(t, t, t));
    }
    else if(l >= inner && l < outer)
    {
        result_color = high_color;
    }
    else if(l >= outer && l < e_max)
    {
        float t = (l - outer) / (e_max - outer);
        t = 3.0 * t*t - 2.0*t * t*t;
        result_color = lerp(high_color, low_color, float3(t, t, t));
    }
    else
    {
        discard;
    }
    return float4(result_color, 1.0);
}
