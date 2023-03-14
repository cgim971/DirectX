#pragma once

#include "SkinnedData.h"
#include "../Common/d3dUtil.h"
#include "../Common/MathHelper.h"

using namespace DirectX;
using namespace Microsoft::WRL;

#define MAXLIGHTS 16

enum class RenderLayer : int
{
	Opaque = 0,
	SkinnedOpaque,
	Transparent,
	AlphaTested,
	Debug,
	Skybox,
	Count
};

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Uv;
	XMFLOAT3 Tangent;
};

struct SkinnedVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexC;
	XMFLOAT3 TangentU;
	XMFLOAT3 BoneWeights;
	BYTE BoneIndices[4];
};

// 개별 오브젝트 상수
struct ObjectConstants
{
	XMFLOAT4X4 World = MathHelper::Identity4x4();
	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
};

// 개별 오브젝트의 재질 상수
struct MatConstants
{
	XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float Roughness = 0.25f;
	UINT Texture_On = 0;
	UINT Normal_On = 0;
	XMFLOAT2 padding = { 0.0f, 0.0f };
};

// 조명 정보 구조체
struct LightInfo
{
	UINT LightTyp = 0;
	XMFLOAT3 padding = { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 Strength = { 0.5f, 0.5f, 0.5f };
	float FalloffStart = 1.0f;
	XMFLOAT3 Direction = { 0.0f, -1.0f, 0.0f };
	float FalloffEnd = 10.0f;
	XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };
	float SpotPower = 64.0f;
};

// 공용 사용하는 상수
struct PassConstants
{
	XMFLOAT4X4 View = MathHelper::Identity4x4();
	XMFLOAT4X4 InvView = MathHelper::Identity4x4();
	XMFLOAT4X4 Proj = MathHelper::Identity4x4();
	XMFLOAT4X4 InvProj = MathHelper::Identity4x4();
	XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();
	XMFLOAT4X4 InvViewProj = MathHelper::Identity4x4();
	XMFLOAT4X4 ShadowTransform = MathHelper::Identity4x4();

	// 조명 정보
	XMFLOAT4 AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };
	XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
	UINT LightCount = MAXLIGHTS;
	LightInfo Lights[MAXLIGHTS];

	// 안개 효과 정보
	XMFLOAT4 FogColor = { 0.7f, 0.7f, 0.7f, 1.0f };
	float gFogStart = 5.0f;
	float gFogRange = 150.0f;
	XMFLOAT2 FogPadding;
};

struct SkinnedConstants
{
	XMFLOAT4X4 BoneTransforms[96];
};

struct GeometryInfo
{
	std::string Name;

	// 정점 버퍼 뷰
	D3D12_VERTEX_BUFFER_VIEW                VertexView = { };
	ComPtr<ID3D12Resource>                  VertexBuffer = nullptr;

	// 인덱스 버퍼 뷰
	D3D12_INDEX_BUFFER_VIEW                 IndexView = { };
	ComPtr<ID3D12Resource>                  IndexBuffer = nullptr;

	// 정점의 갯수
	int VertexCount = 0;
	// 인덱스의 갯수
	int IndexCount = 0;

	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;
};

// 텍스처 구조체
struct TextureInfo
{
	// Unique material name for lookup.
	std::string Name;

	std::wstring Filename;

	ComPtr<ID3D12Resource> Resource = nullptr;
	ComPtr<ID3D12Resource> UploadHeap = nullptr;
};

// 재질 구조체
struct MaterialInfo
{
	std::string Name;

	int MatCBIndex = -1;
	int DiffuseSrvHeapIndex = -1;
	int NormalSrvHeapIndex = -1;

	XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float Roughness = 0.25f;
};

struct SkinnedModelInstance
{
	SkinnedData* SkinnedInfo = nullptr;
	std::vector<DirectX::XMFLOAT4X4> FinalTransforms;
	std::string ClipName;
	float TimePos = 0.0f;

	// Called every frame and increments the time position, interpolates the 
	// animations for each bone based on the current animation clip, and 
	// generates the final transforms which are ultimately set to the effect
	// for processing in the vertex shader.
	void UpdateSkinnedAnimation(float dt)
	{
		TimePos += dt;

		// Loop animation
		if (TimePos > SkinnedInfo->GetClipEndTime(ClipName))
			TimePos = 0.0f;

		// Compute the final transforms for this time position.
		SkinnedInfo->GetFinalTransforms(ClipName, TimePos, FinalTransforms);
	}
};

// 오브젝트 구조체
struct RenderItem
{
	RenderItem() = default;

	UINT ObjCBIndex = -1;

	XMFLOAT4X4 World = MathHelper::Identity4x4();
	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// 기하 정보
	GeometryInfo* Geo = nullptr;
	MaterialInfo* Mat = nullptr;

	// Only applicable to skinned render-items.
	UINT SkinnedCBIndex = -1;

	// nullptr if this render-item is not animated by skinned mesh.
	SkinnedModelInstance* SkinnedModelInst = nullptr;
};