#define DIRECTIONINPUT_VERSION
#include "Windows.h"
#include <dinput.h>
#include <wrl.h>

#pragma once

//入力
class Input {

public:
//namespace省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public:

void Initialize(HINSTANCE hInstance,HWND hwnd);

void Update();

private:

//キーボードのデバイス
	Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard;

};