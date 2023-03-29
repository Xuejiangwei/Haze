#pragma once

#include "HazeDebug.h"

#define HAZE_STRING_STREAM std::wstringstream
#define HAZE_OFSTREAM std::wofstream
#define HAZE_IFSTREAM std::wifstream
#define HAZE_STRING std::wstring
#define HAZE_CHAR wchar_t

#define HAZE_BINARY_STRING std::string
#define HAZE_BINARY_OFSTREAM std::ofstream
#define HAZE_BINARY_IFSTREAM std::ifstream
#define HAZE_BINARY_CHAR char
#define HAZE_BINARY_OP_WRITE_CODE_SIZE(X) (const char*)(&X), sizeof(X)
#define HAZE_BINARY_OP_WRITE_CODE(X) (const char*)(&X)
#define HAZE_BINARY_OP_READ_CODE_SIZE(X) (char*)(&X), sizeof(X)

#define HAZE_TEXT(S) L##S

#define HAZE_CAST_VALUE_TYPE(X) (unsigned int)(X)

#define HAZE_CLASS_THIS					HAZE_TEXT("己")
#define HAZE_CLASS_POINTER_ATTR			HAZE_TEXT("指之")
#define HAZE_CLASS_ATTR					HAZE_TEXT("之")

#define HAZE_MAIN_FUNCTION_TEXT			HAZE_TEXT("主函数")

#define ADD_REGISTER					HAZE_TEXT("Add_R")
#define SUB_REGISTER					HAZE_TEXT("Sub_R")
#define MUL_REGISTER					HAZE_TEXT("Mul_R")
#define DIV_REGISTER					HAZE_TEXT("Div_R")

#define RET_REGISTER					HAZE_TEXT("Ret_R")

#define NEW_REGISTER					HAZE_TEXT("New_R")

#define HAZE_CALL_PUSH_ADDRESS_NAME		HAZE_STRING(HAZE_TEXT("Address"))

#define HAZE_CONBINE_CLASS_FUNCTION(CLASS, FUNCTION) CLASS##FUNCTION

#define HAZE_I_CODE_ENABLE			1
#define HAZE_OP_CODE_ENABLE			1

#define HAZE_VM_STACK_SIZE 1024 * 1024 * 20

#define HAZE_STD_CALL_PARAM class HazeStack* Stack, struct FunctionData* Data

#define HAZE_ADDRESS_SIZE (int)sizeof(int)


using uint = unsigned int;