
// fxc.exe /nologo /T vs_5_0 /E vs /O3 /WX /Zpc /Ges /Fh src\platform_win32\shaders\basic_vshader.h /Vn basic_vshader /Qstrip_reflect /Qstrip_debug /Qstrip_priv src\platform_win32\shaders\basic.hlsl
// fxc.exe /nologo /T ps_5_0 /E ps /O3 /WX /Zpc /Ges /Fh src\platform_win32\shaders\basic_pshader.h /Vn basic_pshader /Qstrip_reflect /Qstrip_debug /Qstrip_priv src\platform_win32\shaders\basic.hlsl

struct VS_INPUT
{
     float2 pos : POSITION0;
     float2 tex_coord : TEXCOORD0;
     float4 instance_xform0 : POSITION1;
     float4 instance_xform1 : POSITION2;
     float4 instance_xform2 : POSITION3;
     float4 instance_color : COLOR0;
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

PS_INPUT vs(VS_INPUT input)
{
    PS_INPUT output;
    float4 res = float4(
        dot(float4(input.pos, 0.0f, 1.0f), input.instance_xform0),
        dot(float4(input.pos, 0.0f, 1.0f), input.instance_xform1),
        dot(float4(input.pos, 0.0f, 1.0f), input.instance_xform2),
        1.0f);
    output.pos = mul(utransform, res);
    output.color = input.instance_color;
    return output;
}

float4 ps(PS_INPUT input) : SV_TARGET
{
    return input.color;
}
