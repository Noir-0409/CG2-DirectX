#include "Logger.h"
#include <dxgidebug.h>
#include "StringUtility.h"

namespace Logger
{

	//Log("Complete createD3D12Device!!\n");

	void Log(const std::string & message) { OutputDebugStringA(message.c_str()); }

	void Log(const std::wstring & message) { OutputDebugStringA(StringUtility::ConvertString(message).c_str()); }

}
