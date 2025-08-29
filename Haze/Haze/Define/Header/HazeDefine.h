#pragma once

#define HAZE_STRING_STREAM std::wstringstream
#define HAZE_OFSTREAM std::wofstream
#define HAZE_IFSTREAM std::wifstream

using HString = HazeString; //std::wstring;
using STDString = std::wstring;
using HStringView = std::wstring_view;

#define HAZE_BINARY_STRING std::string
#define HAZE_BINARY_OFSTREAM std::ofstream
#define HAZE_BINARY_IFSTREAM std::ifstream

#define HAZE_ENDL std::endl
#define HAZE_ENDL_D HAZE_ENDL << HAZE_ENDL

#define HAZE_BINARY_CHAR char
#define HAZE_WRITE_AND_SIZE(X) (const char*)(&X), sizeof(X)
#define HAZE_READ(X) (char*)(&X), sizeof(X)

#define HAZE_TO_STR(V) std::to_string(V)
#define HAZE_TO_HAZE_STR(V) std::to_wstring(V)

#define HAZE_TEXT(S) L##S
#define H_TEXT(S) HAZE_TEXT(S)

#define HAZE_ADDRESS_TYPE x_int64
#define HAZE_ADDRESS_SIZE (int)sizeof(HAZE_ADDRESS_TYPE)

#define HAZE_CONBINE_CLASS_FUNCTION(CLASS, FUNCTION) CLASS##FUNCTION


#define HAZE_VERSION					H_TEXT("0.0.1")

#define HAZE_THIRD_LIB_FOLDER			H_TEXT("库")
#define HAZE_THIRD_DLL_LIB_FOLDER		H_TEXT("动态库")
#define HAZE_FILE_SUFFIX				H_TEXT(".hz")
#define HAZE_FILE_INTER_SUFFIX			H_TEXT(".Hzic")
#define HAZE_FILE_INTER					H_TEXT("Intermediate\\")
#define HAZE_FILE_PATH_BIN				H_TEXT("Bin\\")
#define HAZE_FILE_MAIN_BIN				H_TEXT("Main.Hzb")
#define HAZE_MODULE_PATH_CONBINE		H_TEXT("·")
//#define HAZE_INTER_SYMBOL_TABLE			H_TEXT("@@中间符号")
#define HAZE_TYPE_INFO_TABLE			H_TEXT("@@类型信息")

#define HAZE_LOAD_DLL_SUFFIX			H_TEXT(".dll")

#define HAZE_CONSTANT_STRING_NAME		H_TEXT("常字符串指针")
#define HAZE_MULTI_PARAM_NAME			H_TEXT("多参数")

#define HEADER_IMPORT_MODULE			H_TEXT("ImportModuleTable")
#define HEADER_IMPORT_MODULE_MODULE		H_TEXT("ImportModule")

#define HEADER_REF_TYPE_ID				H_TEXT("RefrenceTypeId")

#define HEADER_STRING_GLOBAL_DATA		H_TEXT("GlobalDataTable")

#define HEADER_STRING_STRING_TABLE		H_TEXT("StringTable")
#define HEADER_STRING_ENUM_TABLE		H_TEXT("EnumTable")
#define HEADER_STRING_CLASS_TABLE		H_TEXT("ClassTable")
#define HEADER_STRING_FUNCTION_TABLE	H_TEXT("FunctionTable")

#define CLASS_LABEL_HEADER				H_TEXT("Class")
#define CLASS_FUNCTION_LABEL_HEADER		H_TEXT("CFunction")
#define FUNCTION_LABEL_HEADER			H_TEXT("Function")
#define FUNCTION_PARAM_HEADER			H_TEXT("Param")
#define FUNCTION_START_HEADER			H_TEXT("FunctionStart")
#define FUNCTION_END_HEADER				H_TEXT("FunctionEnd")

#define BLOCK_FLOW_HEADER				H_TEXT("BlockFlows")

#define CLOSURE_NAME_PREFIX				H_TEXT("Closure_")
#define CLOSURE_REF_VARIABLE			H_TEXT("ClosureRef")

#define ENUM_START_HEADER				H_TEXT("EnumStart")
#define ENUM_END_HEADER					H_TEXT("EnumEnd")

//#define SYMBOL_BEGIN					H_TEXT("SymbolBegin")
//#define SYMBOL_END						H_TEXT("SymbolEnd")

#define TYPE_INFO_FUNC_PARAM_BEGIN		H_TEXT("TypeInfoFunctionBegin")
#define TYPE_INFO_FUNC_PARAM_END		H_TEXT("TypeInfoFunctionEnd")
#define TYPE_INFO_BEGIN					H_TEXT("TypeInfoBegin")
#define TYPE_INFO_END					H_TEXT("TypeInfoEnd")

#define HAZE_LOCAL_VARIABLE_HEADER		H_TEXT("Variable")
#define HAZE_LOCAL_VARIABLE_CONBINE		H_TEXT("$")

#define HAZE_LOCAL_TEMP_REGISTER_HEADER H_TEXT("TempRegister")
#define HAZE_LOCAL_TEMP_REGISTER		H_TEXT("TempR")

#define HAZE_GLOBAL_DATA_INIT_FUNCTION	H_TEXT("@GlobalDataInit")
#define HAZE_CLASS_FUNCTION_CONBINE		H_TEXT("@")

#define HAZE_TEMPLATE_CONBINE			H_TEXT("&")

#define BLOCK_ENTRY_NAME				H_TEXT("Entry")
#define BLOCK_START						H_TEXT("Block")

#define BLOCK_DEFAULT					H_TEXT("DefaultBlock")
#define BLOCK_IF_THEN					H_TEXT("IfThenBlock")
#define BLOCK_ELSE						H_TEXT("ElseBlock")
#define BLOCK_LOOP						H_TEXT("LoopBlock")
#define BLOCK_WHILE						H_TEXT("WhileBlock")
#define BLOCK_FOR						H_TEXT("ForBlock")
#define BLOCK_FOR_CONDITION				H_TEXT("ForConditionBlock")
#define BLOCK_FOR_STEP					H_TEXT("ForStepBlock")

//#define RET_REGISTER					H_TEXT("Ret_R")
//#define NEW_REGISTER					H_TEXT("New_R")
//#define CMP_REGISTER					H_TEXT("Cmp_R")

#define NULL_PTR						H_TEXT("NULL_PTR")

#define TEMP_REGISTER_A					H_TEXT("Temp_RA")
#define TEMP_REGISTER_B					H_TEXT("Temp_RB")

//#define TEMP_REGISTER_0					H_TEXT("Temp_R0")
//#define TEMP_REGISTER_1					H_TEXT("Temp_R1")
//#define TEMP_REGISTER_2					H_TEXT("Temp_R2")
//#define TEMP_REGISTER_3					H_TEXT("Temp_R3")
//#define TEMP_REGISTER_4					H_TEXT("Temp_R4")
//#define TEMP_REGISTER_5					H_TEXT("Temp_R5")
//#define TEMP_REGISTER_6					H_TEXT("Temp_R6")
//#define TEMP_REGISTER_7					H_TEXT("Temp_R7")
//#define TEMP_REGISTER_8					H_TEXT("Temp_R8")
//#define TEMP_REGISTER_9					H_TEXT("Temp_R9")

#define	HAZE_JMP_NULL					H_TEXT("JmpNull")

#define HAZE_CALL_PUSH_ADDRESS_NAME		STDString(H_TEXT("RetPC"))
#define HAZE_CALL_PUSH_ADDRESS_TYPE		(HAZE_ADDRESS_SIZE == 8 ? HazeValueType::Int64 : HAZE_ADDRESS_SIZE == 4 ? HazeValueType::Int32 : HazeValueType::None)

#define HAZE_OBJECT_ARRAY_CONSTRUCTOR	H_TEXT("多对象构造")
#define HAZE_ADVANCE_GET_FUNCTION		H_TEXT("获得")
#define HAZE_ADVANCE_SET_FUNCTION		H_TEXT("设置")
#define HAZE_OBJECT_BASE_CONSTRUCTOR	H_TEXT("@基本对象构造")

#define HAZE_CUSTOM_GET_MEMBER			H_TEXT("获得")
#define HAZE_CUSTOM_SET_MEMBER			H_TEXT("设置")
#define HAZE_CUSTOM_CALL_FUNCTION		H_TEXT("调用")

#define HAZE_CLOSURE_NAME				H_TEXT("@@ClosureCaller")

#define HAZE_STD_CALL_PARAM			class HazeStack* stack, int multiParamNum, int paramByteSize
#define HAZE_OBJECT_CALL_PARAM		class HazeStack* stack, int multiParamNum, int& paramByteSize
#define HAZE_STD_CALL_PARAM_VAR		stack, multiParamNum, paramByteSize

#define HAZE_VM_STACK_SIZE 1024 * 1024 * 4

#define HAZE_NEW_ALIGN_BYTE 4

#define HAZE_ALIGN_BYTE 4

#define HAZE_ALIGN(X, ALIGN) ((X + ALIGN -1) & ~(ALIGN -1))

#define HAZE_NEW_ALIGN(X) ((X + HAZE_NEW_ALIGN_BYTE -1) & ~(HAZE_NEW_ALIGN_BYTE -1))
