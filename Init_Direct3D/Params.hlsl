#ifndef _PARAMS_HLSLI_
#define _PARAMS_HLSLI_

#define MAXLIGHTS 16

struct Light
{
	int lightType;
	float3 padding;
	float3 Strength;
	float FalloffStart;
	float3 Direction;
	float FalloffEnd;
	float3 Position;
	float SpotPower;
};

struct Material
{
	float4 DiffuseAlbedo;
	float3 FresnelR0;
	float Shininess;
};

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorld;
	float4x4 gTexTransform;
};

cbuffer cbMaterial : register(b1)
{
	float4 gDiffuseAlbedo;
	float3 gFresnelR0;
	float gRoughness;
	int gTex_on;
	int gNormal_on;
	float2 gTexPadding;
};

cbuffer cbPass : register(b2)
{
	float4x4 gView;
	float4x4 gInvView;
	float4x4 gProj;
	float4x4 gInvProj;
	float4x4 gViewProj;
	float4x4 gInvViewProj;
	float4x4 gShadowTransform;

	float4 gAmbientLight;
	float3 gEyePosW;
	int gLightCount;
	Light gLights[MAXLIGHTS];

	float4 gFogColor;
	float gFogStart;
	float gFogRange;
	float2 FogPadding;
};

cbuffer cbSkinned : register(b3)
{
	float4x4 gBoneTransforms[96];
};

TextureCube	 gCubeMap	: register(t0);
Texture2D    gTexture_0 : register(t1);
Texture2D    gNormal_0 : register(t2);
Texture2D    gShadowMap : register(t3);

SamplerState gSampler_0 : register(s0);
SamplerComparisonState gsamShadow : register(s1);


// Transform a normal map sample to world space
float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW)
{
	float3 normalT = 2.0f * normalMapSample - 1.0f;

	float3 N = unitNormalW;
	float3 T = normalize(tangentW - dot(tangentW, N) * N);
	float3 B = cross(N, T);

	float3x3 TBN = float3x3(T, B, N);

	float3 bumpedNormalW = mul(normalT, TBN);

	return bumpedNormalW;
}

//---------------------------------------------------------------------------------------
// PCF for shadow mapping.
//---------------------------------------------------------------------------------------

float CalcShadowFactor(float4 shadowPosH)
{
	// Complete projection by doing division by w.
	shadowPosH.xyz /= shadowPosH.w;

	// Depth in NDC space.
	float depth = shadowPosH.z;

	uint width, height, numMips;
	gShadowMap.GetDimensions(0, width, height, numMips);

	// Texel size.
	float dx = 1.0f / (float)width;

	float percentLit = 0.0f;
	const float2 offsets[9] =
	{
		float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
	};

	[unroll]
	for (int i = 0; i < 9; ++i)
	{
		percentLit += gShadowMap.SampleCmpLevelZero(gsamShadow,
			shadowPosH.xy + offsets[i], depth).r;
	}

	return percentLit / 9.0f;
}

#endif 