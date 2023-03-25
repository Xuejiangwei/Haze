#pragma once

#include <unordered_map>
#include <sstream>
#include <string>

#include "HazeDefine.h"
#include "HazeStrcut.h"
#include "HazeValue.h"
#include "HazeToken.h"
#include "HazeInstruction.h"

extern HazeValueType GetStrongerType(HazeValueType Type1, HazeValueType Type2);

extern HazeValueType GetValueTypeByToken(HazeToken Token);

extern void StringToHazeValueNumber(const HAZE_STRING& Str, HazeValue& Value);

extern unsigned int GetSizeByType(HazeValueType Type);

extern bool IsNumber(const HAZE_STRING& Str);

extern HazeValueType GetNumberDefaultType(const HAZE_STRING& Str);

/*
 * Generate file header string
 */
extern const HAZE_CHAR* GetGlobalDataHeaderString();

extern const HAZE_CHAR* GetStringTableHeaderString();

extern const HAZE_CHAR* GetClassTableHeaderString();

extern const HAZE_CHAR* GetClassLabelHeader();

extern const HAZE_CHAR* GetFucntionTableHeaderString();

extern const HAZE_CHAR* GetFunctionLabelHeader();

extern const HAZE_CHAR* GetFunctionParamHeader();

extern const HAZE_CHAR* GetFunctionStartHeader();

extern const HAZE_CHAR* GetFunctionEndHeader();

template <typename T>
T StringToStandardType(const HAZE_STRING& String);

/*
 * Instruction string
 */
extern const HAZE_CHAR* GetInstructionString(InstructionOpCode Code);
extern InstructionOpCode GetInstructionByString(const HAZE_STRING& String);

extern HAZE_STRING String2WString(std::string& Str);

extern std::string WString2String(std::wstring& Str);

extern HAZE_BINARY_CHAR* GetBinaryPointer(HazeValue& Value);

//using InstructionData = std::pair<InstructionScopeType, std::pair<HAZE_STRING, unsigned int>>; // <名字, <global:在符号表中的索引,local:数据类型>>

#include "HazeTemplate.inl"

