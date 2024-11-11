#pragma once
#include "Windows.h"
#include <wrl.h>
#include <cstdint>

//WindowsAPI
class WinApp {

public:

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public:

	void Initialize();

	void Update();

	//終了
	void Finalize();

	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;

	//ゲッター
	HWND GetHwnd() const { return hwnd; }
	HINSTANCE GetHInstance() const { return wc.hInstance; }

private:

	//ウィンドウハンドル
	HWND hwnd = nullptr;	

	//ウィンドウクラスの設定
	WNDCLASS wc{};

};