#pragma once

#include <vector>
#include <string>
#include <sstream>

#include "HazeDefine.h"
#include "HazeStrcut.h"
#include "HazeValue.h"


bool IsAndOrToken(HazeToken Token);

bool IsAndToken(HazeToken Token);

bool IsOrToken(HazeToken Token);

const HAZE_CHAR* GetGlobalDataHeaderString();

const HAZE_CHAR* GetStringTableHeaderString();

const HAZE_CHAR* GetClassTableHeaderString();

const HAZE_CHAR* GetClassLabelHeader();

const HAZE_CHAR* GetFucntionTableHeaderString();

const HAZE_CHAR* GetFunctionLabelHeader();

const HAZE_CHAR* GetFunctionParamHeader();

const HAZE_CHAR* GetFunctionVariableHeader();

const HAZE_CHAR* GetFunctionStartHeader();

const HAZE_CHAR* GetFunctionEndHeader();

bool HazeIsSpace(HAZE_CHAR Char, bool* IsNewLine = nullptr);

bool IsNumber(const HAZE_STRING& Str);

HazeValueType GetNumberDefaultType(const HAZE_STRING& Str);

HAZE_STRING String2WString(const char* Str);

HAZE_STRING String2WString(const HAZE_BINARY_STRING& Str);

HAZE_BINARY_STRING WString2String(const HAZE_STRING& Str);

char* UTF8_2_GB2312(const char* utf8);

char* GB2312_2_UFT8(const char* gb2312);

template <typename T>
unsigned int GetSizeByType(HazeDefineType Type, T* This);

template <typename T>
T StringToStandardType(const HAZE_STRING& String);

template <typename T>
T StringToStandardType(const HAZE_CHAR* String);

template <typename T>
HAZE_BINARY_STRING ToString(T Value);

template <typename T>
HAZE_STRING ToHazeString(T Value);

template <typename T>
T GetHazeValueByBaseType(const char* Address, HazeValueType Type);

#include "HazeTemplate.inl"
