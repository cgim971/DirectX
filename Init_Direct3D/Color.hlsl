#ifndef _COLOR_HLSL_
#define _COLOR_HLSL_

#include "Params.hlsl"
#include "LightingUtil.hlsl"

struct VertexIn
{
    float3 PosL     : POSITION;
    float3 NormalL  : NORMAL;
    float2 Uv       : TEXCOORD;
    float3 Tangent  : TANGENT;
#ifdef SKINNED
    float3 BoneWeights : WEIGHTS;
    uint4 BoneIndices  : BONEINDICES;
#endif
};

struct VertexOut
{
    float4 PosH     : SV_POSITION;
    float4 ShadowPosH : POSITION0;
    float3 PosW     : POSITION1;
    float3 NormalW  : NORMAL;
    float3 TangentW : TANGENT;
    float2 Uv       : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;

#ifdef SKINNED
    float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    weights[0] = vin.BoneWeights.x;
    weights[1] = vin.BoneWeights.y;
    weights[2] = vin.BoneWeights.z;
    weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

    float3 posL = float3(0.0f, 0.0f, 0.0f);
    float3 normalL = float3(0.0f, 0.0f, 0.0f);
    float3 tangentL = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 4; ++i)
    {
        // Assume no nonuniform scaling when transforming normals, so 
        // that we do not have to use the inverse-transpose.

        posL += weights[i] * mul(float4(vin.PosL, 1.0f), gBoneTransforms[vin.BoneIndices[i]]).xyz;
        normalL += weights[i] * mul(vin.NormalL, (float3x3)gBoneTransforms[vin.BoneIndices[i]]);
        tangentL += weights[i] * mul(vin.Tangent.xyz, (float3x3)gBoneTransforms[vin.BoneIndices[i]]);
    }

    vin.PosL = posL;
    vin.NormalL = normalL;
    vin.Tangent.xyz = tangentL;
#endif

    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
    vout.PosH = mul(posW, gViewProj);

    vout.PosW = posW.xyz;

    vout.NormalW = mul(vin.NormalL, (float3x3)gWorld);

    vout.TangentW = mul(vin.Tangent, (float3x3)gWorld);

    float4 Uv = mul(float4(vin.Uv, 0.0f, 1.0f), gTexTransform);
    vout.Uv = Uv.xy;

    // Generate projective tex-coords to project shadow map onto scene.
    vout.ShadowPosH = mul(posW, gShadowTransform);

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    float4 diffuseAlbedo = gDiffuseAlbedo;

    if (gTex_on)
        diffuseAlbedo = gTexture_0.Sample(gSampler_0, pin.Uv) * gDiffuseAlbedo;

#ifdef ALPHA_TEST
    // Discard pixel if texture alpha < 0.1.  We do this test as soon 
    // as possible in the shader so that we can potentially exit the
    // shader early, thereby skipping the rest of the shader code.
    clip(diffuseAlbedo.a - 0.1f);
#endif

    pin.NormalW = normalize(pin.NormalW);

    // 노말 텍스처 계산
    float4 normalMapSample;
    float3 bumpedNormalW = pin.NormalW;
    if (gNormal_on)
    {
        normalMapSample = gNormal_0.Sample(gSampler_0, pin.Uv);
        bumpedNormalW = NormalSampleToWorldSpace(normalMapSample.rgb, pin.NormalW, pin.TangentW);
    }

    float3 toEyeW = normalize(gEyePosW - pin.PosW);

    float4 ambient = gAmbientLight * diffuseAlbedo;

    // Only the first light casts a shadow.
    float shadowFactor = float3(1.0f, 1.0f, 1.0f);
    shadowFactor = CalcShadowFactor(pin.ShadowPosH);

    const float shininess = 1.0f - gRoughness;

    Material mat = { diffuseAlbedo, gFresnelR0, shininess };

    float4 directLight = ComputeLighting(gLights, gLightCount, mat, pin.PosW, bumpedNormalW, toEyeW, shadowFactor);

    float4 litColor = ambient + directLight;

    // 환경 매핑
    float3 r = reflect(-toEyeW, bumpedNormalW);
    float4 reflectionColor = gCubeMap.Sample(gSampler_0, r);
    float3 fresnelFactor = SchlickFresnel(gFresnelR0, bumpedNormalW, r);
    litColor.rgb += shininess * fresnelFactor * reflectionColor;

#ifdef FOG
    float distToEye = length(gEyePosW - pin.PosW);

    float fogAmount = saturate((distToEye - gFogStart) / gFogRange);
    litColor = lerp(litColor, gFogColor, fogAmount);
#endif

    litColor.a = diffuseAlbedo.a;
    return litColor;
}

#endif