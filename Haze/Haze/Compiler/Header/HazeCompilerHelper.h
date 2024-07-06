#pragma once

#include "HazeHeader.h"

class HazeCompilerModule;
class HazeCompilerValue;
class HazeCompilerFunction;
class HazeCompilerClass;

HString GetLocalVariableName(const HString& name, Share<HazeCompilerValue> value);

void HazeCompilerStream(HAZE_STRING_STREAM& hss, HazeCompilerValue* value, bool streamValue = true);

void HazeCompilerStream(HAZE_STRING_STREAM& hss, Share<HazeCompilerValue> value, bool streamValue = true);

Share<HazeCompilerValue> CreateVariable(HazeCompilerModule* compilerModule, const HazeDefineVariable& var, 
	HazeVariableScope scope, HazeDataDesc desc, int count, Share<HazeCompilerValue> refValue = nullptr,
	V_Array<Share<HazeCompilerValue>> arraySize = {}, V_Array<HazeDefineType>* params = nullptr);

V_Array<Pair<HazeDataDesc, V_Array<Share<HazeCompilerValue>>>> CreateVariableCopyClassMember(
	HazeCompilerModule* compilerModule, HazeVariableScope scope, HazeCompilerClass* compilerClass);

void StreamCompilerValue(HAZE_STRING_STREAM& hss, InstructionOpCode insCode, Share<HazeCompilerValue> value, 
	const HChar* defaultName = nullptr);

HString GetObjectName(const HString& inName);

Share<HazeCompilerValue> GetObjectMember(HazeCompilerModule* compilerModule, const HString& inName);

Share<HazeCompilerValue> GetObjectMember(HazeCompilerModule* compilerModule, const HString& inName, bool& isPointer);

Share<HazeCompilerValue> GetObjectNameAndMemberName(HazeCompilerModule* compilerModule, const HString& inName,
	HString& outObjectName, HString& outMemberName, bool& isPointer);

Share<HazeCompilerFunction> GetObjectFunction(HazeCompilerModule* compilerModule, const HString& inName);

Pair<Share<HazeCompilerFunction>, Share<HazeCompilerValue>> GetObjectFunction(
	HazeCompilerModule* compilerModule, const HString& inName, bool& isPointer);

Pair<Share<HazeCompilerFunction>, Share<HazeCompilerValue>> GetObjectNameAndFunctionName(
	HazeCompilerModule* compilerModule, const HString& inName, HString& outObjectName, HString& outFunctionName, bool& isPointer);

bool TrtGetVariableName(HazeCompilerFunction* function, const Pair<HString, Share<HazeCompilerValue>>& data,
	const Share<HazeCompilerValue>& value, HString& outName);

bool TrtGetVariableName(HazeCompilerFunction* function, const Pair<HString, Share<HazeCompilerValue>>& data,
	const HazeCompilerValue* value, HString& outName);

Share<HazeCompilerValue> GetArrayElementToValue(HazeCompilerModule* compilerModule,
	Share<HazeCompilerValue> elementValue, Share<HazeCompilerValue> movToValue = nullptr);

void GetTemplateClassName(HString& inName, const V_Array<HazeDefineType>& templateTypes);
