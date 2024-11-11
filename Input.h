#pragma once

#define DIRECTIONINPUT_VERSION
#include "Windows.h"
#include <dinput.h>
#include <wrl.h>
#include "WinApp.h"

//入力
class Input {

//DirectInputの初期化
Microsoft::WRL::ComPtr<IDirectInput8> directInput = nullptr;

public:
//namespace省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public:

void Initialize(WinApp* winApp);

void Update();

bool PushKey(BYTE KeyNumber);

bool TriggerKey(BYTE KeyNumber);

private:

//キーボードのデバイス
	Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard;

//全キーの状態
	BYTE key[256] = {};
	BYTE keyPre[256] = {};

//WindowsApi
	WinApp* winApp = nullptr;

};