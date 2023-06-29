#pragma once

#define EXE_FUNC_ERR -1

#ifndef HAZE

extern "C" __declspec(dllexport) int ExecuteFunction(const wchar_t* FunctionName, char* ParamStartAddress, char* RetStartAddress);

#define GET_PARAM_START() int Offset = 0
#define GET_PARAM(V, Address)  memcpy(&V, Address - sizeof(V) - Offset, sizeof(V)); Offset += sizeof(V)
#define SET_RET(V, Address) memcpy(Address, &V, sizeof(V))

#else

using ExeFuncType = int(*)(const wchar_t*, char*, char*);

#endif // HAZE
