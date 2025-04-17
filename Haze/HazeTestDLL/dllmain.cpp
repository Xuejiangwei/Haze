// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"

#include "HazePch.h"
#include "HazeLibraryDefine.h"
#include "HazeValue.h"
#include "HazeDefine.h"
#include "HazeStack.h"
#include "HazeInstruction.h"

//BOOL APIENTRY DllMain( HMODULE hModule,
//                       DWORD  ul_reason_for_call,
//                       LPVOID lpReserved
//                     )
//{
//    switch (ul_reason_for_call)
//    {
//    case DLL_PROCESS_ATTACH:
//    case DLL_THREAD_ATTACH:
//    case DLL_THREAD_DETACH:
//    case DLL_PROCESS_DETACH:
//        break;
//    }
//    return TRUE;
//}

int ExecuteFunction(const wchar_t* functionName, char* paramStartAddress, char* retStartAddress, void* stack, void(*exeHazeFunction)(void*, void*, int, ...))
{
	if (functionName == HString(H_TEXT("加法")))
	{
		/*GET_PARAM_START();
		int a, b;
		GET_PARAM(a);
		GET_PARAM(b);

		a = a + b;

		SET_RET_BY_TYPE(HazeValueType::Int32, a);*/
	}

	return 0;
}