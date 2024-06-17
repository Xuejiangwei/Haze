#pragma once

#include <vector>
#include <string>
#include <sstream>

#include "HazeDefine.h"
#include "HazeStrcut.h"
#include "HazeValue.h"
#include "HazeLibraryType.h"
#include "HazeInstruction.h"

bool IsAndOrToken(HazeToken token);

bool IsAndToken(HazeToken token);

bool IsOrToken(HazeToken token);

bool IsCanCastToken(HazeToken token);

int Log2(int n);

const HAZE_CHAR* GetGlobalDataHeaderString();

const HAZE_CHAR* GetGlobalDataInitBlockStart();

const HAZE_CHAR* GetGlobalDataInitBlockEnd();

const HAZE_CHAR* GetStringTableHeaderString();

const HAZE_CHAR* GetClassTableHeaderString();

const HAZE_CHAR* GetClassLabelHeader();

const HAZE_CHAR* GetFucntionTableHeaderString();

const HAZE_CHAR* GetFunctionLabelHeader();

const HAZE_CHAR* GetFunctionParamHeader();

const HAZE_CHAR* GetFunctionVariableHeader();

const HAZE_CHAR* GetFunctionStartHeader();

const HAZE_CHAR* GetFunctionEndHeader();

bool HazeIsSpace(HAZE_CHAR hChar, bool* isNewLine = nullptr);

bool IsNumber(const HAZE_STRING& str);

HazeValueType GetNumberDefaultType(const HAZE_STRING& str);

HAZE_STRING String2WString(const char* str);

HAZE_STRING String2WString(const HAZE_BINARY_STRING& str);

HAZE_BINARY_STRING WString2String(const HAZE_STRING& str);

char* UTF8_2_GB2312(const char* utf8);

char* GB2312_2_UFT8(const char* gb2312);

void ReplacePathSlash(HAZE_STRING& path);

HazeLibraryType GetHazeLibraryTypeByToken(HazeToken token);

InstructionFunctionType GetFunctionTypeByLibraryType(HazeLibraryType type);

HAZE_STRING GetModuleNameByFilePath(const HAZE_STRING& filePath);

template <typename T>
unsigned int GetSizeByType(HazeDefineType type, T* This);

template <typename T>
unsigned int GetNewAllocSizeByType(HazeDefineType type, T* This);

template <typename T>
T StringToStandardType(const HAZE_STRING& str);

template <typename T>
T StringToStandardType(const HAZE_CHAR* str);

template <typename T>
HAZE_BINARY_STRING ToString(T value);

HAZE_BINARY_STRING ToString(void* value);

template <typename T>
HAZE_STRING ToHazeString(T value);

void ConvertBaseTypeValue(HazeValueType type1, HazeValue& v1, HazeValueType type2, const HazeValue& v2);

#include "HazeTemplate.inl"
