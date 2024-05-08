#include <Windows.h>
#include <cstdint>
#include <string>
#include <format>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <dxgidebug.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")

std::wstring ConvertString(const std::string& str) {
	if (str.empty()) {
		return std::wstring();
	}

	auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
	if (sizeNeeded == 0) {
		return std::wstring();
	}
	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
	return result;
}

std::string ConvertString(const std::wstring& str) {
	if (str.empty()) {
		return std::string();
	}

	auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
	if (sizeNeeded == 0) {
		return std::string();
	}
	std::string result(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
	return result;
}

void Log(const std::string& message) {

	OutputDebugStringA(message.c_str());

}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

	switch (msg) {

	case WM_DESTROY:

		PostQuitMessage(0);

		return 0;

	}

	return DefWindowProc(hwnd, msg, wparam, lparam);

}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

#pragma region Windowの生成

	WNDCLASS wc{};

	wc.lpfnWndProc = WindowProc;

	wc.lpszClassName = L"CG2WindowClass";

	wc.hInstance = GetModuleHandle(nullptr);

	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	RegisterClass(&wc);

	const int32_t kClientWidth = 1280;
	const int32_t kClientHeight = 720;

	RECT wrc = { 0,0,kClientWidth,kClientHeight };

	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	HWND hwnd = CreateWindow(
		wc.lpszClassName,
		L"CG2",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		wc.hInstance,
		nullptr);

	ShowWindow(hwnd, SW_SHOW);

#pragma endregion

	#ifdef _DEBUG

	ID3D12Debug1* debugController = nullptr;

	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {

		debugController->EnableDebugLayer();

		debugController->SetEnableGPUBasedValidation(TRUE);

	}

	#endif

#pragma region Factoryの生成

	IDXGIFactory7* dxgiFactory = nullptr;

	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));

	assert(SUCCEEDED(hr));

#pragma endregion

#pragma region Adapterの生成

	IDXGIAdapter4* useAdapter = nullptr;

	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference
	(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) !=DXGI_ERROR_NOT_FOUND; ++i) {

		DXGI_ADAPTER_DESC3 adapterDesc{};

		hr = useAdapter->GetDesc3(&adapterDesc);

		assert(SUCCEEDED(hr));

		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {

			Log(ConvertString( std::format(L"Use Adapter:{}\n", adapterDesc.Description)));

			break;

		}

		useAdapter = nullptr;

	}

	assert(useAdapter != nullptr);

#pragma endregion

#pragma region Deviceの作成

	ID3D12Device* device = nullptr;

	D3D_FEATURE_LEVEL featureLevels[] = {

		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0

	};

	const char* featureLevelStrings[] = { "12.2","12.1","12.0" };

	for (size_t i = 0; i < _countof(featureLevels); ++i) {

		hr = D3D12CreateDevice(useAdapter, featureLevels[i], IID_PPV_ARGS(&device));

		if (SUCCEEDED(hr)) {

			Log(std::format("FeatureLevel:{}\n", featureLevelStrings[i]));

			break;

		}

	}

	assert(device != nullptr);

	Log("Complete createD3D12Device!!\n");

#pragma endregion

#ifdef _DEBUG

	ID3D12InfoQueue* infoQueue = nullptr;

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

#pragma region CommandQueueの生成

	ID3D12CommandQueue* commandQueue = nullptr;

	D3D12_COMMAND_QUEUE_DESC commandQueueDesk{};

	hr = device->CreateCommandQueue(&commandQueueDesk,
		IID_PPV_ARGS(&commandQueue));

	assert(SUCCEEDED(hr));

#pragma endregion

#pragma region CommadAllocatorの生成

	ID3D12CommandAllocator* commandAllocator = nullptr;

	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));

	assert(SUCCEEDED(hr));

#pragma endregion

#pragma region CommandListの生成

	ID3D12GraphicsCommandList* commandList = nullptr;

	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr,
		IID_PPV_ARGS(&commandList));

	assert(SUCCEEDED(hr));

#pragma endregion

#pragma region SwapChainの生成

	IDXGISwapChain4* swapChain = nullptr;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};

	swapChainDesc.Width = kClientWidth;

	swapChainDesc.Height = kClientHeight;

	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	swapChainDesc.SampleDesc.Count = 1;

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	swapChainDesc.BufferCount = 2;

	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue, hwnd ,&swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(&swapChain));

	assert(SUCCEEDED(hr));

#pragma endregion

#pragma region DescriptorHeapの生成

	ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc{};

	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	rtvDescriptorHeapDesc.NumDescriptors = 2;

	hr = device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));

	assert(SUCCEEDED(hr));

#pragma endregion

	ID3D12Resource* swapChainResources[2] = { nullptr };

	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));

	assert(SUCCEEDED(hr));

	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));

	assert(SUCCEEDED(hr));

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};

	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];

	rtvHandles[0] = rtvStartHandle;

	device->CreateRenderTargetView(swapChainResources[0], &rtvDesc, rtvHandles[0]);

	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	device->CreateRenderTargetView(swapChainResources[1], &rtvDesc, rtvHandles[1]);

	ID3D12Fence* fence = nullptr;

	uint64_t fenceValue = 0;

	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

	assert(SUCCEEDED(hr));

	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	assert(fenceEvent != nullptr);

	MSG msg{};

	while (msg.message != WM_QUIT) {

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

			TranslateMessage(&msg);
			DispatchMessage(&msg);

		} else {

			UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

			D3D12_RESOURCE_BARRIER barrier{};

			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

			barrier.Transition.pResource = swapChainResources[backBufferIndex];

			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;

			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

			commandList->ResourceBarrier(1, &barrier);

			commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);

			float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };

			commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);

			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;

			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

			commandList->ResourceBarrier(1, &barrier);

			hr = commandList->Close();

			assert(SUCCEEDED(hr));

			ID3D12CommandList* commandLists[] = { commandList };

			commandQueue->ExecuteCommandLists(1, commandLists);

			swapChain->Present(1, 0);

			fenceValue++;

			commandQueue->Signal(fence, fenceValue);

			if (fence->GetCompletedValue() < fenceValue) {

				fence->SetEventOnCompletion(fenceValue, fenceEvent);

				WaitForSingleObject(fenceEvent, INFINITE);

			}

			hr = commandAllocator->Reset();

			assert(SUCCEEDED(hr));

			hr = commandList->Reset(commandAllocator, nullptr);

			assert(SUCCEEDED(hr));

		}
	}

	CloseHandle(fenceEvent);

	fence->Release();

	rtvDescriptorHeap->Release();

	swapChainResources[0]->Release();

	swapChainResources[1]->Release();

	swapChain->Release();

	commandList->Release();

	commandAllocator->Release();

	commandQueue->Release();

	device->Release();

	useAdapter->Release();

	dxgiFactory->Release();

#ifdef _DEBUG

	debugController->Release();

#endif

	CloseWindow(hwnd);

	IDXGIDebug* debug;

	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {

		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);

		debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);

		debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);

		debug->Release();

	}

	Log("Hello,DirectX!\n");

	return 0;

}
