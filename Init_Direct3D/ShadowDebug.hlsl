//***************************************************************************************
// ShadowDebug.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

// Include common HLSL code.
#include "Params.hlsl"

struct VertexIn
{
	float3 PosL    : POSITION;
	float2 Uv    : TEXCOORD;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
	float2 Uv    : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;

    // Already in homogeneous clip space.
    vout.PosH = float4(vin.PosL, 1.0f);
	
	vout.Uv = vin.Uv;
	
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return float4(gShadowMap.Sample(gSampler_0, pin.Uv).rrr, 1.0f);
}


