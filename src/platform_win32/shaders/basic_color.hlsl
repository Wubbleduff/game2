
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
    return output;
}

float4 ps(PS_INPUT input) : SV_TARGET
{
    float3 result_color = input.color.xyz;

    return float4(result_color, 1.0);

}
