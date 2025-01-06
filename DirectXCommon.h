#pragma once
#include <wrl.h>
#include "externals/imgui/imgui_impl_dx12.h"
#include <d3dcommon.h>
#include <d3d12.h>
#include <format>
#include <dxgi1_6.h>
#include "WinApp.h"
#include <dxcapi.h>
#include "externals/DirectXTex/DirectXTex.h"
#include <array>

struct Vector2 {

	float x;
	float y;

};

struct Vector3 {

	float x;
	float y;
	float z;

};

struct Vector4 {

	float x;
	float y;
	float z;
	float w;

};

struct MaterialData {

	std::string textureFilePath;

};

struct Matrix3x3 {

	float m[3][3];

};

struct Matrix4x4 {

	float m[4][4];

};

struct Transform {

	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;

};

struct Material {

	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;

};

struct TransformationMatrix
{
	Matrix4x4 WVP;
	Matrix4x4 World;

};

struct DirectionalLight {

	Vector4 color; // ライトの色
	Vector3 direction; // ライトの向き
	float intensity; // 輝度

};

struct VertexData {

	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;

};

//DirectX基盤
class DirectXCommon {

public:

void Initialize(WinApp* winApp);

void DeviceInitialize();

void CommandInitialize();

void SwapChainCreate();

void DepthBufferCreate();

void DescriptorHeapCreate();

void RTVInitialize();

void DepthStencilViewInitialize();

void FenceCreate();

void ViewPortRectInitialize();

void ScissorRectInitialize();

void DXCCompilerCreate();

void ImGuiInitialize();

Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencialTextureResource( int32_t width, int32_t height);

//SRVの指定番号のCPUデスクリプタハンドルを取得する
D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);

//SRVの指定番号のGPUデスクリプタハンドルを取得する
D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t size);

Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, const DirectX::TexMetadata& metadata);

//スワップチェーンリソース
std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources;

//描画前処理
void PreDraw();

//描画後処理
void PostDraw();

//getter
ID3D12Device* GetDevice() const { return device.Get(); }
ID3D12GraphicsCommandList* GetCommandList() const { return commandList.Get(); }

private:

	HRESULT hr;

	//DirectX12デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	//DXGIファクトリ
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	Microsoft::WRL::ComPtr < ID3D12CommandQueue> commandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain;

	uint32_t descriptorSizeSRV_ = 0;
	uint32_t descriptorSizeRTV_ = 0;
	uint32_t descriptorSizeDSV_ = 0;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;

	Microsoft::WRL::ComPtr < ID3D12Fence> fence;
	D3D12_VIEWPORT viewport{};
	D3D12_RECT scissorRect{};

	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	IDxcIncludeHandler* includeHandler = nullptr;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};

	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle;

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];

	D3D12_RESOURCE_BARRIER barrier{};

	uint64_t fenceValue = 0;

	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	//WindowsAPI
	WinApp* winApp = nullptr;

	//指定番号のCPUデスクリプタハンドルを取得する
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);

	//指定番号のGPUデスクリプタハンドルを取得する
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);

	//シェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile);

};