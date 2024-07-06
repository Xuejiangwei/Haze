#pragma once

#include "HazeDefine.h"
#include "HazeStrcut.h"
#include "HazeValue.h"
#include "HazeLibraryType.h"

bool IsAndOrToken(HazeToken token);

bool IsAndToken(HazeToken token);

bool IsOrToken(HazeToken token);

bool IsCanCastToken(HazeToken token);

int Log2(int n);

HString GetHazeClassFunctionName(const HString& className, const HString& functionName);

const HChar* GetGlobalDataHeaderString();

const HChar* GetGlobalDataInitBlockStart();

const HChar* GetGlobalDataInitBlockEnd();

const HChar* GetStringTableHeaderString();

const HChar* GetClassTableHeaderString();

const HChar* GetClassLabelHeader();

const HChar* GetFucntionTableHeaderString();

const HChar* GetFunctionLabelHeader();

const HChar* GetFunctionParamHeader();

const HChar* GetFunctionVariableHeader();

const HChar* GetFunctionStartHeader();

const HChar* GetFunctionEndHeader();

bool HazeIsSpace(HChar hChar, bool* isNewLine = nullptr);

bool IsNumber(const HString& str);

HazeValueType GetNumberDefaultType(const HString& str);

HString String2WString(const char* str);

HString String2WString(const HAZE_BINARY_STRING& str);

HAZE_BINARY_STRING WString2String(const HString& str);

char* UTF8_2_GB2312(const char* utf8);

char* GB2312_2_UFT8(const char* gb2312);

void ReplacePathSlash(HString& path);

HazeLibraryType GetHazeLibraryTypeByToken(HazeToken token);

enum class InstructionFunctionType GetFunctionTypeByLibraryType(HazeLibraryType type);

HString GetModuleNameByFilePath(const HString& filePath);

template <typename T>
unsigned int GetSizeByType(const HazeDefineType& type, T* This);

template <typename T>
unsigned int GetNewAllocSizeByType(const HazeDefineType& type, T* This);

template <typename T>
T StringToStandardType(const HString& str);

template <typename T>
T StringToStandardType(const HChar* str);

template <typename T>
HAZE_BINARY_STRING ToString(T value);

HAZE_BINARY_STRING ToString(void* value);

template <typename T>
HString ToHazeString(T value);

void ConvertBaseTypeValue(HazeValueType type1, HazeValue& v1, HazeValueType type2, const HazeValue& v2);

V_Array<HString> HazeStringSplit(const HString& str, const HString& delimiter);

#include "HazeTemplate.inl"
