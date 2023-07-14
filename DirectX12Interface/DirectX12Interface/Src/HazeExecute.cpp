#include <unordered_map>
#include <string>

#include "DX12Interface.h"

int ExecuteFunction(const wchar_t* FunctionName, char* ParamStartAddress, char* RetStartAddress, void* Stack, void(*ExeHazeFunction)(void*, void*, int, ...))
{
	static std::unordered_map<std::wstring, void(*)(HAZE_CALL_FUNC_PARAM)> HashMap_Interface =
	{
		/*{ L"����Windows����", &CreateWindowsWindow },
		{ L"��ʼ��DirectX", &InitDirect3D },
		{ L"����Haze", &CallHaze },
		{ L"����Windows����", }*/
	};

	auto Iter = HashMap_Interface.find(FunctionName);
	if (Iter != HashMap_Interface.end())
	{
		Iter->second(ParamStartAddress, RetStartAddress, Stack, ExeHazeFunction);
		return 0;
	}

	return EXE_FUNC_ERR;
}