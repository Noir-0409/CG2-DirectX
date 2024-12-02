#pragma once
#include <wrl.h>
#include "externals/imgui/imgui_impl_dx12.h"
#include <d3dcommon.h>
#include <d3d12.h>
#include <format>
#include <dxgi1_6.h>
#include "WinApp.h"

//DirectX基盤
class DirectXCommon {

public:

void Initialize();

void DeviceInitialize();

void CommandInitialize();

void SwapChainCreate();

void ZBufferCreate();

void DescriptorHeapCreate();

void RTVInitialize();

void ZStencilViewInitialize();

void FenceCreate();

void ViewPortRectInitialize();

void ScissorRectInitialize();

void DXCCompilerCreate();

void ImGuiInitialize();

private:

	//DirectX12デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device;

	//DXGIファクトリ
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;

	//WindowsAPI
	WinApp* winApp = nullptr;

};