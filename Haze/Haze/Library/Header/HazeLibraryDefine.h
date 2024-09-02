#pragma once

#define EXE_FUNC_ERR -1

#ifndef HAZE

extern "C" __declspec(dllexport) int ExecuteFunction(const wchar_t* functionName, char* paramStartAddress, char* retStartAddress, void* stack, \
	void(*exeHazeFunction)(void*, void*, int, ...));

#else

using ExeFuncType = int(*)(const wchar_t*, char*, char*, void*, void(*)(void*, void*, int, ...));

#endif // HAZE

#define HAZE_CALL_NULL_PARAM nullptr, nullptr, nullptr, nullptr
#define HAZE_CALL_FUNC_PARAM char* paramStartAddress, char* retStartAddress, void* stack, void(*exeHazeFunction)(void*, void*, int, ...)
#define ARG_T(ARGS) ARGS																	//解决VC编译错误
#define ARG_N(A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15,A16,N,...)  N				//截取并返回第16个参数，这里限制了可变参数计算的范围［1，16］
#define ARG_N_HELPER(...)  ARG_T(ARG_N(__VA_ARGS__))										//辅助宏
#define COUNT_ARG(...)  ARG_N_HELPER(__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0)	//返回可变参数个数

#define GET_PARAM_START() int zzzOffset = 0; auto zzzAddress = stack->GetAddressByESP(HAZE_ADDRESS_SIZE)
#define GET_PARAM(V)  memcpy(&V, zzzAddress - sizeof(V) - zzzOffset, sizeof(V)); zzzOffset += sizeof(V)
#define GET_PARAM_ADDRESS(V, SIZE)  V = zzzAddress - SIZE - zzzOffset; zzzOffset += SIZE
#define SET_RET(V) memcpy(zzzAddress, &V, sizeof(V))
#define GET_RET(V) memcpy(&V, zzzAddress, sizeof(V))
#define SET_HAZE_RET_TO_RET() 0
#define SET_HAZE_CALL_PARAM(...) COUNT_ARG(__VA_ARGS__), __VA_ARGS__

#define SET_RET_BY_TYPE(TYPE, V) \
	HazeRegister* retRegister = stack->GetVirtualRegister(RET_REGISTER); \
	retRegister->Type.PrimaryType = TYPE; \
	retRegister->Data.resize(retRegister->Type.GetTypeSize()); \
	memcpy(retRegister->Data.begin()._Unwrapped(), &V, retRegister->Data.size())