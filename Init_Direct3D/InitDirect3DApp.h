#pragma once

#include "D3dApp.h"
#include "D3dHeader.h"
#include <DirectXColors.h>
#include "../Common/MathHelper.h"
#include "../Common/GeometryGenerator.h"
#include "../Common/DDSTextureLoader.h"
#include "../Common/Camera.h"
#include "ShadowMap.h"
#include "SkinnedData.h"
#include "LoadM3d.h"

class InitDirect3DApp : public D3DApp
{
public:
	InitDirect3DApp(HINSTANCE hInstance);
	~InitDirect3DApp();

	virtual bool Initialize()override;

private:
	virtual void CreateDsvDescriptorHeaps()override;

	virtual void OnResize()override;
	
	virtual void Update(const GameTimer& gt)override;
	void UpdateCamera(const GameTimer& gt);
	void UpdateObjectCBs(const GameTimer& gt);
	void UpdateMaterialCBs(const GameTimer& gt);
	void UpdateSkinnedCBs(const GameTimer& gt);
	void UpdateShadowTransform(const GameTimer& gt);
	void UpdatePassCB(const GameTimer& gt);
	void UpdateShadowPassCB(const GameTimer& gt);

	virtual void Draw(const GameTimer& gt)override;
	void DrawRenderItems(const std::vector<RenderItem*>& ritems);
	void DrawSceneToShadowMap();

	virtual void DrawBegin(const GameTimer& gt)override;
	virtual void DrawEnd(const GameTimer& gt)override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

private:
	// Skinned Model �ε�
	void LoadSkinnedModel();

	// �ؽ�ó �ε�
	void LoadTextures();

	// ���� ���� ����
	void BuildBoxGeometry();
	void BuildGridGeometry();
	void BuildSphereGeometry();
	void BuildCylinderGeometry();
	void BuildQuadGeometry();
	void BuildSkullGeometry();

	// ���� ����
	void BuildMaterials();

	// ������ �� ������ ����
	void BuildRenderItems();

	void BuildInputLayout();
	void BuildShaders();
	void BuildConstantBuffers();
	void BuildRootSignature();
	void BuildDescriptorHeaps();
	void BuildPSO();

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 2> GetStaticSamplers();

private:
	// �Է� ��ġ
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mSkinnedInputLayout;

	// ���� ������Ʈ ��� ����
	ComPtr<ID3D12Resource>	mObjectCB = nullptr;
	BYTE* mObjectMappedData = nullptr;
	UINT mObjectByteSize = 0;

	// ���� ������Ʈ ���� ��� ����
	ComPtr<ID3D12Resource>	mMaterialCB = nullptr;
	BYTE* mMaterialMappedData = nullptr;
	UINT mMaterialByteSize = 0;

	// ���� ��� ����
	ComPtr<ID3D12Resource>	mPassCB = nullptr;
	BYTE* mPassMappedData = nullptr;
	UINT mPassByteSize = 0;

	// Skinned ����
	ComPtr<ID3D12Resource>	mSkinnedCB = nullptr;
	BYTE* mSkinnedMappedData = nullptr;
	UINT mSkinnedByteSize = 0;

	// ��Ʈ �ñ״�ó
	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

	// ���̴� ��
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;

	// ���������������� ������Ʈ ��
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

	// ������ ��
	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;
	UINT mCbvSrvDescriptorSize = 0;

	// ������ �� ������Ʈ ����Ʈ
	std::vector<std::unique_ptr<RenderItem>> mRenderitems;

	// Render items divided by PSO.
	std::vector<RenderItem*> mRitemLayer[(int)RenderLayer::Count];

	// ���� ���� ��
	std::unordered_map<std::string, std::unique_ptr<GeometryInfo>> mGeometries;

	// ���� ���� ��
	std::unordered_map<std::string, std::unique_ptr<MaterialInfo>> mMaterials;

	// �ؽ�ó ��
	std::unordered_map<std::string, std::unique_ptr<TextureInfo>> mTextures;
	
	// ������ ��
	std::unique_ptr<ShadowMap> mShadowMap;

	// ��� ��
	DirectX::BoundingSphere mSceneBounds;

	// ����Ʈ ���
	float mLightNearZ = 0.0f;
	float mLightFarZ = 0.0f;
	XMFLOAT3 mLightPosW;
	XMFLOAT4X4 mLightView = MathHelper::Identity4x4();
	XMFLOAT4X4 mLightProj = MathHelper::Identity4x4();
	XMFLOAT4X4 mShadowTransform = MathHelper::Identity4x4();

	float mLightRotationAngle = 0.0f;
	XMFLOAT3 mBaseLightDirections[3] = {
		XMFLOAT3(0.57735f, -0.57735f, 0.57735f),
		XMFLOAT3(-0.57735f, -0.57735f, 0.57735f),
		XMFLOAT3(0.0f, -0.707f, -0.707f)
	};
	XMFLOAT3 mRotatedLightDirections[3];

	// ��ī�̹ڽ� �ؽ�ó �ε���
	UINT mSkyboxTexHeapIndex = 0;

	// �׸��� �� �ؽ�ó �ε���
	UINT mShadowMapHeapIndex = 0;

	CD3DX12_GPU_DESCRIPTOR_HANDLE mNullSrv;

	UINT mSkinnedSrvHeapStart = 0;
	std::string mSkinnedModelFilename = "..\\Models\\soldier.m3d";
	std::unique_ptr<SkinnedModelInstance> mSkinnedModelInst;
	SkinnedData mSkinnedInfo;
	std::vector<M3DLoader::Subset> mSkinnedSubsets;
	std::vector<M3DLoader::M3dMaterial> mSkinnedMats;
	std::vector<std::string> mSkinnedTextureNames;

	// ī�޶� Ŭ����
	Camera mCamera;

	// ���콺 ��ǥ
	POINT mLastMousePos = { 0, 0 };
};