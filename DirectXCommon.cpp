#include "DirectXCommon.h"
#include <cassert>
#include "Logger.h"
#include "StringUtility.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

using namespace Microsoft::WRL;

void DirectXCommon::Initialize(WinApp* winApp)
{

	//NULL検出
	assert(winApp);

	//メンバ変数に記録
	this->winApp = winApp;

	DeviceInitialize();
	CommandInitialize();
	SwapChainCreate();
	DepthBufferCreate();
	DescriptorHeapCreate();
	RTVInitialize();
	DepthStencilViewInitialize();
	FenceCreate();
	ViewPortRectInitialize();
	ScissorRectInitialize();
	DXCCompilerCreate();
	ImGuiInitialize();

}

void DirectXCommon::DeviceInitialize()
{

#ifdef _DEBUG

	Microsoft::WRL::ComPtr <ID3D12InfoQueue> infoQueue = nullptr;

	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {

		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);

		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);

		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		D3D12_MESSAGE_ID denyIds[] = {

			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE

		};

		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };

		D3D12_INFO_QUEUE_FILTER filter{};

		filter.DenyList.NumIDs = _countof(denyIds);

		filter.DenyList.pIDList = denyIds;

		filter.DenyList.NumSeverities = _countof(severities);

		filter.DenyList.pSeverityList = severities;

		infoQueue->PushStorageFilter(&filter);

		infoQueue->Release();

	}

#endif

#pragma region Factoryの生成

hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));

	assert(SUCCEEDED(hr));

#pragma endregion

#pragma region Adapterの生成

	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter = nullptr;

	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference
	(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; ++i) {

		DXGI_ADAPTER_DESC3 adapterDesc{};

		hr = useAdapter->GetDesc3(&adapterDesc);

		assert(SUCCEEDED(hr));

		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {

			Logger::Log(StringUtility::ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));

			break;

		}

		useAdapter = nullptr;

	}

	assert(useAdapter != nullptr);

#pragma endregion

#pragma region Deviceの作成

	D3D_FEATURE_LEVEL featureLevels[] = {

		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0

	};

	const char* featureLevelStrings[] = { "12.2","12.1","12.0" };

	for (size_t i = 0; i < _countof(featureLevels); ++i) {

		hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));

		if (SUCCEEDED(hr)) {

			Logger::Log(std::format("FeatureLevel:{}\n", featureLevelStrings[i]));

			break;

		}

	}

	assert(device != nullptr);

#pragma endregion

	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};

	descriptionRootSignature.Flags =

		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};

	descriptorRange[0].BaseShaderRegister = 0; // 0から始める
	
	descriptorRange[0].NumDescriptors = 1; // 数は1つ

	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う

	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // offsetを指導計算

	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	hr = D3D12SerializeRootSignature(&descriptionRootSignature,

		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

	if (FAILED(hr)) {

		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));

		assert(false);

	}

}

void DirectXCommon::CommandInitialize()
{

#pragma region CommadAllocatorの生成

	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));

	assert(SUCCEEDED(hr));

#pragma endregion

#pragma region CommandListの生成

	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr,
		IID_PPV_ARGS(&commandList));

	assert(SUCCEEDED(hr));

#pragma endregion

#pragma region CommandQueueの生成

	D3D12_COMMAND_QUEUE_DESC commandQueueDesk{};

	hr = device->CreateCommandQueue(&commandQueueDesk,
		IID_PPV_ARGS(&commandQueue));

	assert(SUCCEEDED(hr));

#pragma endregion

}

void DirectXCommon::SwapChainCreate()
{

#pragma region SwapChainの生成

	swapChainDesc.Width = WinApp::kClientWidth;

	swapChainDesc.Height = WinApp::kClientHeight;

	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	swapChainDesc.SampleDesc.Count = 1;

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	swapChainDesc.BufferCount = 2;

	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), winApp->GetHwnd(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));

	assert(SUCCEEDED(hr));

#pragma endregion

}

void DirectXCommon::DepthBufferCreate()
{

	// DepthStencilTextureをウィンドウのサイズで作成
	depthStencilResource = CreateDepthStencialTextureResource(WinApp::kClientWidth, WinApp::kClientHeight);

}

void DirectXCommon::DescriptorHeapCreate()
{

	// DescriptorSizeを取得しておく
	descriptorSizeSRV_ = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorSizeRTV_ = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	descriptorSizeDSV_ = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	rtvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	srvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
	// DSV用のヒープでディスクリプタの数は1。DSVはShader内で触るものではないから、ShaderVisibleはfalse
	dsvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);


}

void DirectXCommon::RTVInitialize()
{
	// スワップチェーンのリソースを取得
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources[2] = { nullptr };
	HRESULT hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	assert(SUCCEEDED(hr));

	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));

	// RTVの記述子を設定
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	// RTVディスクリプタヒープの開始アドレスを取得
	rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	// ディスクリプタサイズを取得
	UINT rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// 裏表の2つ分
	for (uint32_t i = 0; i < 2; ++i) {
		// RTVハンドルを計算
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = { rtvStartHandle.ptr + i * rtvDescriptorSize };

		// レンダーターゲットビューの生成
		device->CreateRenderTargetView(swapChainResources[i].Get(), &rtvDesc, rtvHandle);
	}

	// 描画先のRTVとSRVを設定
	dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

}

//void DirectXCommon::RTVInitialize()
//{
//
//	Microsoft::WRL::ComPtr < ID3D12Resource>swapChainResources[2] = { nullptr };
//	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
//	assert(SUCCEEDED(hr));
//
//	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
//	assert(SUCCEEDED(hr));
//
//	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
//	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
//	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
//	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
//	rtvHandles[0] = rtvStartHandle;
//
//	//裏表の2つ分
//	for (uint32_t i = 0; i < 2; ++i) {
//
//		//RTVハンドルを取得
//	
//		//レンダーターゲットビューの生成
//		device->CreateRenderTargetView(swapChainResources[0].Get(), &rtvDesc, rtvHandles[0]);
//
//		rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
//
//		device->CreateRenderTargetView(swapChainResources[1].Get(), &rtvDesc, rtvHandles[1]);
//	
//		
//	}
//
//}

void DirectXCommon::DepthStencilViewInitialize()
{

	// DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};

	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // Format。基本的にresourceに合わせる

	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // 2Dtexture

	// DSVHeapの先頭にDSVを作る
	device->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());


}

void DirectXCommon::FenceCreate()
{

	fenceValue = 0;

	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

	assert(SUCCEEDED(hr));

	fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	assert(fenceEvent != nullptr);

}

void DirectXCommon::ViewPortRectInitialize()
{

	viewport.Width = WinApp::kClientWidth;

	viewport.Height = WinApp::kClientHeight;

	viewport.TopLeftX = 0;

	viewport.TopLeftY = 0;

	viewport.MinDepth = 0.0f;

	viewport.MaxDepth = 1.0f;

}

void DirectXCommon::ScissorRectInitialize()
{

	scissorRect.left = 0;

	scissorRect.right = WinApp::kClientWidth;

	scissorRect.top = 0;

}

void DirectXCommon::DXCCompilerCreate()
{

	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));

	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));

	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));

}

void DirectXCommon::ImGuiInitialize()
{

	IMGUI_CHECKVERSION();

	ImGui::CreateContext();

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(winApp->GetHwnd());

	ImGui_ImplDX12_Init(device.Get(),

		swapChainDesc.BufferCount,

		rtvDesc.Format,

		srvDescriptorHeap.Get(),

		srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),

		srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateDepthStencialTextureResource(int32_t width, int32_t height)
{

	//生成するResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};

	resourceDesc.Width = width; // 幅

	resourceDesc.Height = height; // 高さ

	resourceDesc.MipLevels = 1; // mipmapの数

	resourceDesc.DepthOrArraySize = 1; // 奥行

	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // DepthStencilとして利用可能なフォーマット

	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント

	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 2次元

	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // DepthStencilとして使う数字

	// 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};

	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAM上に作る

	//深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};

	depthClearValue.DepthStencil.Depth = 1.0f; //最大値でクリア

	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // フォーマット。Resourceと合わせる

	//Resourceの生成
	Microsoft::WRL::ComPtr < ID3D12Resource> resource = nullptr;

	HRESULT hr = device->CreateCommittedResource(

		&heapProperties, //Heapの設定

		D3D12_HEAP_FLAG_NONE, //Heapの特殊な設定。特になし

		&resourceDesc, //Resourceの設定

		D3D12_RESOURCE_STATE_DEPTH_WRITE, //深度値を書き込む状態にしておく

		&depthClearValue, //Clear最適地

		IID_PPV_ARGS(&resource)); //作成するResourceポインタへのポインタ

	assert(SUCCEEDED(hr));

	return resource;

}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVCPUDescriptorHandle(uint32_t index)
{
	
	return GetCPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV_, index);

}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVGPUDescriptorHandle(uint32_t index)
{
	return GetGPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV_, index);

}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DirectXCommon::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible)
{
	Microsoft::WRL::ComPtr < ID3D12DescriptorHeap> DescriptorHeap = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc{};

	DescriptorHeapDesc.Type = heapType;
	DescriptorHeapDesc.NumDescriptors = numDescriptors;
	DescriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = device->CreateDescriptorHeap(&DescriptorHeapDesc, IID_PPV_ARGS(&DescriptorHeap));
	assert(SUCCEEDED(hr));
	return DescriptorHeap;

}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t size)
{

	D3D12_HEAP_PROPERTIES uploadHeapProperties{};

	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC vertexResourceDesc{};

	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;

	vertexResourceDesc.Width = size;

	vertexResourceDesc.Height = 1;

	vertexResourceDesc.DepthOrArraySize = 1;

	vertexResourceDesc.MipLevels = 1;

	vertexResourceDesc.SampleDesc.Count = 1;

	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	Microsoft::WRL::ComPtr < ID3D12Resource> vertexResource = nullptr;

	HRESULT hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,

		&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,

		IID_PPV_ARGS(&vertexResource));

	assert(SUCCEEDED(hr));

	return vertexResource;

}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, const DirectX::TexMetadata& metadata)
{
	
	// metaDataを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};

	resourceDesc.Width = UINT(metadata.width); // Textureの幅

	resourceDesc.Height = UINT(metadata.height); // Textureの高さ

	resourceDesc.MipLevels = UINT16(metadata.mipLevels); // mipmapの数

	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize); // 奥行きor配列Textureの配列数

	resourceDesc.Format = metadata.format;// Textureのformat

	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント。1固定

	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension); // Textureの次元数

	// 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};

	heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM; // 細かい設定を行う

	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK; // WriteBackポリシーでCPUアクセス可能

	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0; // プロセッサの近くに配置

	// Resourceの生成
	Microsoft::WRL::ComPtr < ID3D12Resource> resource = nullptr;

	HRESULT hr = device->CreateCommittedResource(

		&heapProperties, // Heapの設定

		D3D12_HEAP_FLAG_NONE, // Heapの特殊な設定。特になし

		&resourceDesc, // Resourceの設定

		D3D12_RESOURCE_STATE_GENERIC_READ, // 初回のResourceState

		nullptr, // Clear最適地。使わないためnullptr

		IID_PPV_ARGS(&resource)); // 作成するResourceポインタへのポインタ

	assert(SUCCEEDED(hr));

	return resource;

}

void DirectXCommon::PreDraw()
{

	UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	barrier.Transition.pResource = swapChainResources[backBufferIndex].Get();

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;

	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	commandList->ResourceBarrier(1, &barrier);

	commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);

	// 描画先のRTVとDSVを設定
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };

	//色をクリア
	commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);

	// 指定した深度で画面全体をクリア
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeaps[] = { srvDescriptorHeap };

	commandList->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());

	commandList->RSSetViewports(1, &viewport);

	commandList->RSSetScissorRects(1, &scissorRect);

}

void DirectXCommon::PostDraw()
{

	UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;

	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

	commandList->ResourceBarrier(1, &barrier);

	hr = commandList->Close();

	assert(SUCCEEDED(hr));

	Microsoft::WRL::ComPtr<ID3D12CommandList> commandLists[] = { commandList };

	commandQueue->ExecuteCommandLists(1, commandLists->GetAddressOf());

	swapChain->Present(1, 0);

	fenceValue++;

	commandQueue->Signal(fence.Get(), fenceValue);

	if (fence->GetCompletedValue() < fenceValue) {

		fence->SetEventOnCompletion(fenceValue, fenceEvent);

		WaitForSingleObject(fenceEvent, INFINITE);

	}

	hr = commandAllocator->Reset();

	assert(SUCCEEDED(hr));

	hr = commandList->Reset(commandAllocator.Get(), nullptr);

	assert(SUCCEEDED(hr));

}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
	return D3D12_CPU_DESCRIPTOR_HANDLE();
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
	return D3D12_GPU_DESCRIPTOR_HANDLE();
}
