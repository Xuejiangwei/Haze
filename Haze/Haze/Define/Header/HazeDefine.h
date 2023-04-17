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

#define HAZE_CAST_VALUE_TYPE(X) (uint32)(X)

#define HAZE_CLASS_THIS					HAZE_TEXT("己")
#define HAZE_CLASS_POINTER_ATTR			HAZE_TEXT("指之")
#define HAZE_CLASS_ATTR					HAZE_TEXT("之")

#define HAZE_MAIN_FUNCTION_TEXT			HAZE_TEXT("主函数")

#define HAZE_CONSTANT_STRING_NAME		HAZE_TEXT("常字符串指针")

#define HEADER_STRING_GLOBAL_DATA		HAZE_TEXT("GlobalDataTable")
#define HEADER_STRING_STRING_TABLE		HAZE_TEXT("StringTable")
#define HEADER_STRING_CLASS_TABLE		HAZE_TEXT("ClassTable")
#define HEADER_STRING_FUNCTION_TABLE	HAZE_TEXT("FunctionTable")

#define CLASS_LABEL_HEADER				HAZE_TEXT("Class")
#define FUNCTION_LABEL_HEADER			HAZE_TEXT("Function")
#define FUNCTION_PARAM_HEADER			HAZE_TEXT("Param")
#define FUNCTION_START_HEADER			HAZE_TEXT("Start")
#define FUNCTION_END_HEADER				HAZE_TEXT("End")

#define HAZE_LOCAL_VARIABLE_HEADER		HAZE_TEXT("Variable")
#define HAZE_LOCAL_VARIABLE_CONBINE		HAZE_TEXT("$")

#define BLOCK_ENTRY_NAME				HAZE_TEXT("Entry")
#define BLOCK_START						HAZE_TEXT("Block")

#define ADD_REGISTER					HAZE_TEXT("Add_R")
#define SUB_REGISTER					HAZE_TEXT("Sub_R")
#define MUL_REGISTER					HAZE_TEXT("Mul_R")
#define DIV_REGISTER					HAZE_TEXT("Div_R")

#define RET_REGISTER					HAZE_TEXT("Ret_R")

#define NEW_REGISTER					HAZE_TEXT("New_R")

#define CMP_REGISTER					HAZE_TEXT("Cmp_R")

#define HAZE_TEMP_BINART_NAME			HAZE_TEXT("TempBinaryValue")

#define	HAZE_JMP_NULL					HAZE_TEXT("JmpNull")
#define	HAZE_JMP_OUT					HAZE_TEXT("JmpOut")

#define HAZE_CALL_PUSH_ADDRESS_NAME		HAZE_STRING(HAZE_TEXT("Address"))

#define HAZE_CONBINE_CLASS_FUNCTION(CLASS, FUNCTION) CLASS##FUNCTION

#define HAZE_I_CODE_ENABLE			1

#define HAZE_VM_STACK_SIZE 1024 * 1024 * 20

#define HAZE_STD_CALL_PARAM class HazeStack* Stack, struct FunctionData* Data, int MultiParamNum

#define HAZE_ADDRESS_SIZE (int)sizeof(int)

using int64 = long long;
using uint8 = unsigned char;
using uint32 = unsigned int;
using uint64 = unsigned long long;