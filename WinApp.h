#pragma once
#include "Windows.h"
#include <wrl.h>

//WindowsAPI
class WinApp {

public:

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public:

	void Initialize();

	void Update();

	HWND GetHwnd() const { return hwnd; }

	HINSTANCE GetHInstance() const { return wc.hInstance; }

	const int32_t kClientWidth = 1280;
	const int32_t kClientHeight = 720;

private:

//ウィンドウハンドル
	HWND hwnd;

//ウィンドウクラスの設定
WNDCLASS wc{};

};