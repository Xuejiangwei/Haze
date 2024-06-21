#pragma once

#define HAZE_STRING_STREAM std::wstringstream
#define HAZE_OFSTREAM std::wofstream
#define HAZE_IFSTREAM std::wifstream
#define HAZE_STRING std::wstring
#define HAZE_BYTE char
#define HAZE_CHAR wchar_t

#define HAZE_BINARY_STRING std::string
#define HAZE_BINARY_OFSTREAM std::ofstream
#define HAZE_BINARY_IFSTREAM std::ifstream
#define HAZE_BINARY_CHAR char
#define HAZE_WRITE_AND_SIZE(X) (const char*)(&X), sizeof(X)
#define HAZE_READ(X) (char*)(&X), sizeof(X)

#define HAZE_TO_STR(V) std::to_string(V)
#define HAZE_TO_HAZE_STR(V) std::to_wstring(V)

#define HAZE_TEXT(S) L##S

#define HAZE_ADDRESS_SIZE (int)sizeof(int)

#define HAZE_CONBINE_CLASS_FUNCTION(CLASS, FUNCTION) CLASS##FUNCTION

#define HAZE_STANDARD_FOLDER			HAZE_TEXT("标准库\\")
#define HAZE_FILE_SUFFIX				HAZE_TEXT(".hz")
#define HAZE_FILE_INTER_SUFFIX			HAZE_TEXT(".Hzic")
#define HAZE_FILE_INTER					HAZE_TEXT("Intermediate\\")
#define HAZE_FILE_PATH_BIN				HAZE_TEXT("Bin\\")
#define HAZE_FILE_MAIN_BIN				HAZE_TEXT("Main.Hzb")

#define HAZE_CLASS_THIS					HAZE_TEXT("己")
#define HAZE_CLASS_POINTER_ATTR			HAZE_TEXT("指之")
#define HAZE_CLASS_ATTR					HAZE_TEXT("之")

#define HAZE_DEFAULT_MAIN_FUNCTION_TEXT			HAZE_TEXT("主函数")

#define HAZE_CONSTANT_STRING_NAME		HAZE_TEXT("常字符串指针")

#define HAZE_MULTI_PARAM_NAME			HAZE_TEXT("多参数")

#define HEADER_STRING_GLOBAL_DATA		HAZE_TEXT("GlobalDataTable")
#define GLOBAL_DATA_INIT_BLOCK_START	HAZE_TEXT("GlobalDataInitStart")
#define GLOBAL_DATA_INIT_BLOCK_END		HAZE_TEXT("GlobalDataInitEnd")

#define HEADER_STRING_STRING_TABLE		HAZE_TEXT("StringTable")
#define HEADER_STRING_CLASS_TABLE		HAZE_TEXT("ClassTable")
#define HEADER_STRING_FUNCTION_TABLE	HAZE_TEXT("FunctionTable")

#define CLASS_LABEL_HEADER				HAZE_TEXT("Class")
#define FUNCTION_LABEL_HEADER			HAZE_TEXT("Function")
#define FUNCTION_PARAM_HEADER			HAZE_TEXT("Param")
#define FUNCTION_START_HEADER			HAZE_TEXT("FunctionStart")
#define FUNCTION_END_HEADER				HAZE_TEXT("FunctionEnd")

#define HAZE_LOCAL_VARIABLE_HEADER		HAZE_TEXT("Variable")
#define HAZE_LOCAL_VARIABLE_CONBINE		HAZE_TEXT("$")

#define HAZE_CLASS_FUNCTION_CONBINE		HAZE_TEXT("@")

#define BLOCK_ENTRY_NAME				HAZE_TEXT("Entry")
#define BLOCK_START						HAZE_TEXT("Block")

#define BLOCK_DEFAULT					HAZE_TEXT("DefaultBlock")
#define BLOCK_IF_THEN					HAZE_TEXT("IfThenBlock")
#define BLOCK_ELSE						HAZE_TEXT("ElseBlock")
#define BLOCK_LOOP						HAZE_TEXT("LoopBlock")
#define BLOCK_WHILE						HAZE_TEXT("WhileBlock")
#define BLOCK_FOR						HAZE_TEXT("ForBlock")
#define BLOCK_FOR_CONDITION				HAZE_TEXT("ForConditionBlock")
#define BLOCK_FOR_STEP					HAZE_TEXT("ForStepBlock")

#define ADD_REGISTER					HAZE_TEXT("Add_R")
#define SUB_REGISTER					HAZE_TEXT("Sub_R")
#define MUL_REGISTER					HAZE_TEXT("Mul_R")
#define DIV_REGISTER					HAZE_TEXT("Div_R")

#define RET_REGISTER					HAZE_TEXT("Ret_R")
#define NEW_REGISTER					HAZE_TEXT("New_R")
#define CMP_REGISTER					HAZE_TEXT("Cmp_R")

#define NULL_PTR						HAZE_TEXT("NULL_PTR")

#define TEMP_REGISTER_0					HAZE_TEXT("Temp_R0")
#define TEMP_REGISTER_1					HAZE_TEXT("Temp_R1")
#define TEMP_REGISTER_2					HAZE_TEXT("Temp_R2")
#define TEMP_REGISTER_3					HAZE_TEXT("Temp_R3")
#define TEMP_REGISTER_4					HAZE_TEXT("Temp_R4")
#define TEMP_REGISTER_5					HAZE_TEXT("Temp_R5")
#define TEMP_REGISTER_6					HAZE_TEXT("Temp_R6")
#define TEMP_REGISTER_7					HAZE_TEXT("Temp_R7")
#define TEMP_REGISTER_8					HAZE_TEXT("Temp_R8")
#define TEMP_REGISTER_9					HAZE_TEXT("Temp_R9")

#define	HAZE_JMP_NULL					HAZE_TEXT("JmpNull")

#define HAZE_CALL_PUSH_ADDRESS_NAME		HAZE_STRING(HAZE_TEXT("RetAddress"))

#define HAZE_OBJECT_ARRAY_CONSTRUCTOR	HAZE_TEXT("多对象构造")

#define HAZE_STD_CALL_PARAM class HazeStack* stack, struct FunctionData* data, int multiParamNum

#define HAZE_VM_STACK_SIZE 1024 * 1024 * 20

#define HAZE_NEW_ALIGN_BYTE 4

#define HAZE_ALIGN_BYTE 4

#define HAZE_ALIGN(X, ALIGN) ((X + ALIGN -1) & ~(ALIGN -1))

#define HAZE_NEW_ALIGN(X) ((X + HAZE_NEW_ALIGN_BYTE -1) & ~(HAZE_NEW_ALIGN_BYTE -1))

using uint8 = unsigned char;
using uchar = unsigned char;
using hbyte = HAZE_BYTE;
using uhbyte = unsigned HAZE_BYTE;
using hchar = HAZE_CHAR;
using ushort = unsigned short;
using uint32 = unsigned int;
using int64 = long long;
using uint64 = unsigned long long;
using usize = size_t;